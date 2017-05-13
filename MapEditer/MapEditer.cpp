#include "stdafx.h"
#include "InputLayer.h"
#include "Definition.h"
#include "MyTimer.h"
#include "MapEditer.h"

namespace DirectXFramework
{
	const int tempWidth = 800;
	const int tempHeight = 600;

	MapEditer::MapEditer(HINSTANCE hInstance, int nCmdShow)
		: m_hInstance(hInstance),
		m_CmdShow(nCmdShow),
		m_Width(tempWidth),
		m_Height(tempHeight)
	{
		InitWindow();
		InitDirectX();

		m_pInputLayer = new InputLayer;
		m_pInputLayer->Initialize();

		m_pTimer = new MyTimer;
		m_pTimer->Init();

		CreateShader();
		CreateVertexBuffer();
		CreateIndexBuffer();
		CreateConstantBuffer();
		CreateRenderState(D3D11_FILL_SOLID, D3D11_CULL_BACK);

		InitMatrix();
		LoadTexture();

		// WndProc�� ����� �� �ֵ��� ������ ���.
		mapEditerHandler = this;
	}

	MapEditer::~MapEditer()
	{
		CleanUpDevice();
		DestroyWindow(m_hWnd);
		mapEditerHandler = nullptr;
	}

	void MapEditer::Run()
	{
		static float AccTime = 0.f;
		m_pTimer->ProcessTime();

		while (true)
		{
			AccTime += m_pTimer->GetElapsedTime();
			if (PeekMessage(&m_Message, NULL, 0, 0, PM_REMOVE))
			{
				if (m_Message.message == WM_QUIT) break;
				TranslateMessage(&m_Message);
				DispatchMessage(&m_Message);
			}
			else
			{
				if (AccTime > 0.0167f)
				{
					if (!CalcProc(AccTime)) break;
					if (!DrawProc(AccTime)) break;
					AccTime = 0.f;
				}
			}
		}
	}

	void MapEditer::CleanUpDevice()
	{
		if (m_pImmediateContext) m_pImmediateContext->ClearState();

		if (m_pSolidRS)          m_pSolidRS->Release();
		if (m_pSamplerLinear)	 m_pSamplerLinear->Release();
		if (m_pTextureRV)		 m_pTextureRV->Release();

		if (m_pDepthStencilView) m_pDepthStencilView->Release();
		if (m_pConstantBuffer)   m_pConstantBuffer->Release();
		if (m_pIndexBuffer)      m_pIndexBuffer->Release();

		if (m_pVertexBuffer)     m_pVertexBuffer->Release();
		if (m_pVertexLayout)     m_pVertexLayout->Release();
		if (m_pVertexShader)     m_pVertexShader->Release();
		if (m_pPixelShader)      m_pPixelShader->Release();

		if (m_pRenderTargetView) m_pRenderTargetView->Release();
		if (m_pSwapChain)        m_pSwapChain->Release();
		if (m_pImmediateContext) m_pImmediateContext->Release();
		if (m_pD3DDevice)        m_pD3DDevice->Release();
	}

	bool MapEditer::InitWindow()
	{
		WNDCLASS WndClass;
		WndClass.cbClsExtra = 0;
		WndClass.cbWndExtra = 0;
		WndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
		WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
		WndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
		WndClass.hInstance = m_hInstance;
		WndClass.lpfnWndProc = WndProc;
		WndClass.lpszClassName = m_AppName;
		WndClass.lpszMenuName = NULL;
		WndClass.style = CS_HREDRAW | CS_VREDRAW;

		if (!RegisterClass(&WndClass)) return false;

		m_hWnd = CreateWindow(
			m_AppName, m_AppName, WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT, CW_USEDEFAULT, m_Width, m_Height,
			NULL, (HMENU)NULL, m_hInstance, NULL);

		ShowWindow(m_hWnd, m_CmdShow);
		return true;
	}

	bool MapEditer::CreateDeviceAndSwapChain()
	{
		UINT createDeviceFlags = 0;
#ifdef _DEBUG
		createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif	
		D3D_FEATURE_LEVEL     featureLevels[] =
		{
			D3D_FEATURE_LEVEL_11_0,
			D3D_FEATURE_LEVEL_10_1,
			D3D_FEATURE_LEVEL_10_0,
		};
		UINT numFeatureLevels = ARRAYSIZE(featureLevels);

		DXGI_SWAP_CHAIN_DESC sd;
		ZeroMemory(&sd, sizeof(sd));
		sd.BufferCount = 1;

		sd.BufferDesc.Width = m_Width;
		sd.BufferDesc.Height = m_Height;
		sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		sd.BufferDesc.RefreshRate.Numerator = 60;
		sd.BufferDesc.RefreshRate.Denominator = 1;

		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sd.OutputWindow = m_hWnd;
		sd.SampleDesc.Count = 1;
		sd.SampleDesc.Quality = 0;
		sd.Windowed = TRUE;

		auto hr = D3D11CreateDeviceAndSwapChain(
			0,
			D3D_DRIVER_TYPE_HARDWARE,
			0,
			createDeviceFlags,
			featureLevels,
			numFeatureLevels,
			D3D11_SDK_VERSION,
			&sd,
			&m_pSwapChain,
			&m_pD3DDevice,
			&m_FeatureLevel,
			&m_pImmediateContext);

		if (FAILED(hr)) return false;

		return true;
	}

	bool MapEditer::CreateRenderTargetView()
	{
		ID3D11Texture2D* pBackBuffer = NULL;
		auto hr = m_pSwapChain->GetBuffer(0,
			__uuidof(ID3D11Texture2D),
			(LPVOID*)&pBackBuffer);

		if (FAILED(hr)) return false;

		hr = m_pD3DDevice->CreateRenderTargetView(pBackBuffer,
			NULL,
			&m_pRenderTargetView);
		pBackBuffer->Release();

		if (FAILED(hr)) return false;

		m_pImmediateContext->OMSetRenderTargets(
			1,
			&m_pRenderTargetView,
			NULL);

		return true;
	}

	bool MapEditer::CreateViewPort()
	{
		D3D11_VIEWPORT vp;
		vp.Width = (FLOAT)m_Width;
		vp.Height = (FLOAT)m_Height;
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
		vp.TopLeftX = 0;
		vp.TopLeftY = 0;
		m_pImmediateContext->RSSetViewports(1, &vp);
		return true;
	}

	bool MapEditer::InitDirectX()
	{
		if (!CreateDeviceAndSwapChain()) return false;
		if (!CreateRenderTargetView()) return false;
		if (!CreateViewPort()) return false;
		if (!CreateDepthStencilTexture()) return false;

		return true;
	}

	bool MapEditer::CreateShader()
	{
		ID3DBlob   *pErrorBlob = NULL;
		ID3DBlob   *pVSBlob = NULL;
		auto hr = D3DX11CompileFromFile(
			L"MyShader.fx", 0, 0,
			"VS", "vs_5_0", 0, 0, 0,
			&pVSBlob, &pErrorBlob, 0);

		if (FAILED(hr)) return false;

		hr = m_pD3DDevice->CreateVertexShader(
			pVSBlob->GetBufferPointer(),
			pVSBlob->GetBufferSize(),
			0,
			&m_pVertexShader);

		if (FAILED(hr)) return false;

		D3D11_INPUT_ELEMENT_DESC layout[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};

		UINT   numElements = ARRAYSIZE(layout);
		hr = m_pD3DDevice->CreateInputLayout(
			layout,
			numElements,
			pVSBlob->GetBufferPointer(),
			pVSBlob->GetBufferSize(),
			&m_pVertexLayout);
		pVSBlob->Release();

		if (FAILED(hr)) return false;

		ID3DBlob *pPSBlob = NULL;
		D3DX11CompileFromFile(
			L"MyShader.fx", 0, 0,
			"PS", "ps_5_0", 0, 0, 0,
			&pPSBlob, &pErrorBlob, 0);

		m_pD3DDevice->CreatePixelShader(
			pPSBlob->GetBufferPointer(),
			pPSBlob->GetBufferSize(),
			0,
			&m_pPixelShader);
		pPSBlob->Release();

		return true;
	}

	bool MapEditer::CreateVertexBuffer()
	{
		MyVertex vertices[] =
		{
			{ XMFLOAT3(-1.0f, 1.0f, -1.0f),  XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f), XMFLOAT3(-0.33f, 0.33f, -0.33f) , XMFLOAT2(1.0f, 1.0f) },
			{ XMFLOAT3(1.0f, 1.0f, -1.0f),   XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f), XMFLOAT3(0.33f, 0.33f, -0.33f) , XMFLOAT2(0.0f, 1.0f) },
			{ XMFLOAT3(1.0f, 1.0f, 1.0f),    XMFLOAT4(0.0f, 1.0f, 1.0f, 1.0f), XMFLOAT3(0.33f, 0.33f, 0.33f) , XMFLOAT2(0.0f, 0.0f) },
			{ XMFLOAT3(-1.0f, 1.0f, 1.0f),   XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f), XMFLOAT3(-0.33f, 0.33f, 0.33f) , XMFLOAT2(1.0f, 0.0f) },
			{ XMFLOAT3(-1.0f, -1.0f, -1.0f), XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f), XMFLOAT3(-0.33f, -0.33f, -0.33f) , XMFLOAT2(0.0f, 0.0f) },
			{ XMFLOAT3(1.0f, -1.0f, -1.0f),  XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f), XMFLOAT3(0.33f, -0.33f, -0.33f) , XMFLOAT2(1.0f, 0.0f) },
			{ XMFLOAT3(1.0f, -1.0f, 1.0f),   XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), XMFLOAT3(0.33f, -0.33f, 0.33f) , XMFLOAT2(1.0f, 1.0f) },
			{ XMFLOAT3(-1.0f, -1.0f, 1.0f),  XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f), XMFLOAT3(-0.33f, -0.33f, 0.33f) , XMFLOAT2(0.0f, 1.0f) },
		};

		D3D11_BUFFER_DESC    bd;
		ZeroMemory(&bd, sizeof(bd));
		bd.ByteWidth = sizeof(vertices);
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = 0;

		D3D11_SUBRESOURCE_DATA    initData;
		ZeroMemory(&initData, sizeof(initData));
		initData.pSysMem = vertices;
		m_pD3DDevice->CreateBuffer(
			&bd,
			&initData,
			&m_pVertexBuffer);

		return true;
	}

	bool MapEditer::CreateIndexBuffer()
	{
		UINT     indices[] =
		{
			3, 1, 0,
			2, 1, 3,
			0, 5, 4,
			1, 5, 0,
			3, 4, 7,
			0, 4, 3,
			1, 6, 5,
			2, 6, 1,
			2, 7, 6,
			3, 7, 2,
			6, 4, 5,
			7, 4, 6,
		};

		D3D11_BUFFER_DESC 	ibd;
		ZeroMemory(&ibd, sizeof(ibd));
		ibd.ByteWidth = sizeof(indices);       // sizeof(UINT) * 6;
		ibd.Usage = D3D11_USAGE_IMMUTABLE;
		ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		ibd.CPUAccessFlags = 0;

		D3D11_SUBRESOURCE_DATA 		iinitData;
		ZeroMemory(&iinitData, sizeof(iinitData));
		iinitData.pSysMem = indices;
		m_pD3DDevice->CreateBuffer(&ibd, &iinitData, &m_pIndexBuffer);
		return true;
	}

	bool MapEditer::CreateDepthStencilTexture()
	{
		// Create depth stencil texture
		D3D11_TEXTURE2D_DESC descDepth;
		ZeroMemory(&descDepth, sizeof(descDepth));
		descDepth.Width = m_Width;
		descDepth.Height = m_Height;
		descDepth.MipLevels = 1;
		descDepth.ArraySize = 1;
		descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		descDepth.SampleDesc.Count = 1;
		descDepth.SampleDesc.Quality = 0;
		descDepth.Usage = D3D11_USAGE_DEFAULT;
		descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		descDepth.CPUAccessFlags = 0;
		descDepth.MiscFlags = 0;
		auto hr = m_pD3DDevice->CreateTexture2D(&descDepth, NULL, &m_pDepthStencil);

		if (FAILED(hr)) return false;

		// Create the depth stencil view
		D3D11_DEPTH_STENCIL_VIEW_DESC 	descDSV;
		ZeroMemory(&descDSV, sizeof(descDSV));
		descDSV.Format = descDepth.Format; // == DXGI_FORMAT_D24_UNORM_S8_UINT
		descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;

		// MSAA�� ����Ѵٸ� D3D11_DSV_DIMENSION_TEXTURE2DMS�� �����
		descDSV.Texture2D.MipSlice = 0;
		descDSV.Flags = 0;
		hr = m_pD3DDevice->CreateDepthStencilView(m_pDepthStencil, &descDSV, &m_pDepthStencilView);

		if (FAILED(hr)) return false;

		m_pImmediateContext->OMSetRenderTargets(1, &m_pRenderTargetView, m_pDepthStencilView);

		return true;
	}

	bool MapEditer::CreateRenderState(D3D11_FILL_MODE fillMode, D3D11_CULL_MODE cullMode)
	{
		D3D11_RASTERIZER_DESC      rasterizerDesc;
		ZeroMemory(&rasterizerDesc, sizeof(D3D11_RASTERIZER_DESC));
		rasterizerDesc.FillMode = fillMode;		// Fill �ɼ�
		rasterizerDesc.CullMode = cullMode;	// Culling �ɼ�
		rasterizerDesc.FrontCounterClockwise = false;	  // ��/�޸� ���� ���� CCW
														  // �ݽð� ������ �ո����� �� ���ΰ�?
		auto hr = m_pD3DDevice->CreateRasterizerState(&rasterizerDesc, &m_pSolidRS);

		if (FAILED(hr)) return false;
		return true;
	}

	bool MapEditer::CalcProc(float deltaTime)
	{
		return true;
	}

	void MapEditer::CalculateMatrixForBox(float deltaTime)
	{
		static float accTime = 0;

		accTime += deltaTime / (FLOAT)100;
		// �ڽ��� ȸ����Ű�� ���� ����.    ��ġ, ũ�⸦ �����ϰ��� �Ѵٸ� SRT�� ����� ��.
		XMMATRIX mat = XMMatrixRotationY(accTime);
		mat *= XMMatrixRotationX(-accTime);
		m_World = mat;                     // ���⼭ g_world�� �ڽ��� ���� Matrix��.

		XMMATRIX wvp = m_World * m_View * m_Projection;

		ConstantBuffer cb;
		cb.wvp = XMMatrixTranspose(wvp);

		cb.world = XMMatrixTranspose(m_World);
		cb.lightDir = m_LightDirection;
		cb.lightColor = m_LightColor;

		m_pImmediateContext->UpdateSubresource(m_pConstantBuffer, 0, 0, &cb, 0, 0); // update data
		m_pImmediateContext->VSSetConstantBuffers(0, 1, &m_pConstantBuffer);// set constant buffer.

		m_pImmediateContext->PSSetConstantBuffers(0, 1, &m_pConstantBuffer);
	}

	void MapEditer::CalculateMatrixForBox2(float deltaTime)
	{
		static float accTime = 0.f;

		accTime += deltaTime / 15;
		XMMATRIX   scale = XMMatrixScaling(1.0f, 1.0f, 1.0f);   // scale
		XMMATRIX   rotate = XMMatrixRotationZ(accTime);    // rotate
		float   moveValue = 5.0f;// move position
		XMMATRIX   position = XMMatrixTranslation(moveValue, 0.0f, 0.0f);
		m_World2 = scale  *  rotate  *  position;     // S * R * T

		XMMATRIX   rotate2 = XMMatrixRotationY(-accTime);    // rotate
		m_World2 *= rotate2;

		XMMATRIX   wvp = m_World2  *  m_View  *  m_Projection;
		ConstantBuffer cb;
		cb.wvp = XMMatrixTranspose(wvp);

		cb.world = XMMatrixTranspose(m_World);
		cb.lightDir = m_LightDirection;
		cb.lightColor = m_LightColor;

		m_pImmediateContext->UpdateSubresource(m_pConstantBuffer, 0, 0, &cb, 0, 0);
		m_pImmediateContext->VSSetConstantBuffers(0, 1, &m_pConstantBuffer);

		m_pImmediateContext->PSSetConstantBuffers(0, 1, &m_pConstantBuffer);
	}

	bool MapEditer::DrawProc(float deltaTime)
	{
		float clearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f };

		m_pImmediateContext->ClearRenderTargetView(m_pRenderTargetView, clearColor);
		m_pImmediateContext->ClearDepthStencilView(
			m_pDepthStencilView,
			D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
			1.0f,
			0);

		// Set Input Assembler 
		m_pImmediateContext->IASetInputLayout(m_pVertexLayout);
		m_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		UINT stride = sizeof(MyVertex);
		UINT offset = 0;
		m_pImmediateContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);
		m_pImmediateContext->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

		// Set Shader and Draw
		m_pImmediateContext->VSSetShader(m_pVertexShader, NULL, 0);
		m_pImmediateContext->PSSetShader(m_pPixelShader, NULL, 0);

		m_pImmediateContext->PSSetShaderResources(0, 1, &m_pTextureRV);
		m_pImmediateContext->PSSetSamplers(0, 1, &m_pSamplerLinear);
		m_pImmediateContext->RSSetState(m_pSolidRS);

		CalculateMatrixForBox(deltaTime);
		m_pImmediateContext->DrawIndexed(36, 0, 0);

		//CalculateMatrixForBox2(deltaTime);
		//m_pImmediateContext->DrawIndexed(36, 0, 0);

		// Render (����۸� ����Ʈ���۷� �׸���.)
		m_pSwapChain->Present(0, 0);
		return true;
	}

	void MapEditer::InitMatrix()
	{
		m_World = XMMatrixIdentity();

		// View ��� ����
		XMVECTOR pos = XMVectorSet(0.0f, 0.0f, -8.0f, 1.0f);
		XMVECTOR target = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
		XMVECTOR up = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
		m_View = XMMatrixLookAtLH(pos, target, up);

		m_Projection = XMMatrixPerspectiveFovLH(
			XM_PIDIV2,  	// pi
			800.0f / (FLOAT)600.0f,  // aspect ratio
			0.3f, 1000.0f);  	// near plane, far plane
	}

	void MapEditer::CreateConstantBuffer()
	{
		D3D11_BUFFER_DESC 	cbd;
		ZeroMemory(&cbd, sizeof(cbd));
		cbd.Usage = D3D11_USAGE_DEFAULT;
		cbd.ByteWidth = sizeof(ConstantBuffer);
		cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		cbd.CPUAccessFlags = 0;
		m_pD3DDevice->CreateBuffer(&cbd, NULL, &m_pConstantBuffer);
	}

	bool MapEditer::LoadTexture()
	{
		auto hr = D3DX11CreateShaderResourceViewFromFile(
			m_pD3DDevice,
			L"./images.jpg",
			NULL,
			NULL,
			&m_pTextureRV,
			NULL);

		if (FAILED(hr)) return false;

		D3D11_SAMPLER_DESC 	sampDesc;
		ZeroMemory(&sampDesc, sizeof(sampDesc));
		sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;   // ���� ���� �� ���� ���͸�.
		sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;   // U��ǥ Address Mode
		sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;   // V��ǥ Address Mode
		sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;  // W��ǥ Address Mode
		sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER; // ���ø� ������ �� ����
		sampDesc.MinLOD = 0;			// �ּ� Mipmap Range
		sampDesc.MaxLOD = D3D11_FLOAT32_MAX;	// �ִ� Mipmap Range

		hr = m_pD3DDevice->CreateSamplerState(&sampDesc, &m_pSamplerLinear); // SamplerState ����
		if (FAILED(hr))	return false;

		return true;
	}

	/*
		���� �����ִ� �޽����� ���ؼ� ó�����ִ� �ݹ� �Լ�.
	*/
	LRESULT CALLBACK MapEditer::MessageHandler(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
	{
		switch (iMessage)
		{
		case WM_KEYDOWN:
		{
			m_pInputLayer->KeyDown((unsigned int)wParam);
			return 0;
		}

		case WM_KEYUP:
		{
			m_pInputLayer->KeyUp((unsigned int)lParam);
			return 0;
		}

		default:
			return (DefWindowProc(hWnd, iMessage, wParam, lParam));
		}
	}

	LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
	{
		switch (iMessage)
		{
		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;

		case WM_CLOSE:
			PostQuitMessage(0);
			return 0;

		default:
			return mapEditerHandler->MessageHandler(hWnd, iMessage, wParam, lParam);
		}
	}

}