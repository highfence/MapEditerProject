#include "stdafx.h"
#include "InputLayer.h"
#include "Definition.h"
#include "MyTimer.h"
#include "Camera.h"
#include "GeometryGenerator.h"
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

		MakeInnerObjects();
		DirectXSetting();

		InitMatrix();
		LoadTexture();

		// WndProc이 사용할 수 있도록 포인터 등록.
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
		if (m_pFX)				 m_pFX->Release();
		if (m_pImmediateContext) m_pImmediateContext->ClearState();

		if (m_pSolidRS)          m_pSolidRS->Release();
		if (m_pSamplerLinear)	 m_pSamplerLinear->Release();
		if (m_pTextureRV)		 m_pTextureRV->Release();

		if (m_pDepthStencilView) m_pDepthStencilView->Release();
		if (m_pConstantBuffer)   m_pConstantBuffer->Release();
		if (m_pHeightMapIndexBuffer) m_pHeightMapIndexBuffer->Release();
		if (m_pHeightMapVertexBuffer) m_pHeightMapVertexBuffer->Release();
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

		hr = m_pD3DDevice->CreateRenderTargetView(
			pBackBuffer,
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

	bool MapEditer::DirectXSetting()
	{
		bool retval = true;
		retval = retval && BuildGeometryBuffers();
		retval = retval && CreateShader();
		//retval = retval && CreateVertexBuffer();
		//retval = retval && CreateIndexBuffer();
		retval = retval && CreateConstantBuffer();
		retval = retval && CreateRenderState(
			D3D11_FILL_SOLID,
			D3D11_CULL_BACK);
		return true;
	}

	bool MapEditer::CreateShader()
	{
		ID3DBlob * pErrorBlob = nullptr;
		ID3DBlob * pVSBlob = nullptr;
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

		UINT numElements = ARRAYSIZE(layout);
		hr = m_pD3DDevice->CreateInputLayout(
			layout,
			numElements,
			pVSBlob->GetBufferPointer(),
			pVSBlob->GetBufferSize(),
			&m_pVertexLayout);

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

		D3D11_BUFFER_DESC bd;
		ZeroMemory(&bd, sizeof(bd));
		bd.ByteWidth = sizeof(vertices);
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = 0;

		D3D11_SUBRESOURCE_DATA initData;
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
		UINT indices[] =
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

		D3D11_BUFFER_DESC ibd;
		ZeroMemory(&ibd, sizeof(ibd));
		ibd.ByteWidth = sizeof(indices);       // sizeof(UINT) * 6;
		ibd.Usage = D3D11_USAGE_IMMUTABLE;
		ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		ibd.CPUAccessFlags = 0;

		D3D11_SUBRESOURCE_DATA iinitData;
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
		D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
		ZeroMemory(&descDSV, sizeof(descDSV));
		descDSV.Format = descDepth.Format; // == DXGI_FORMAT_D24_UNORM_S8_UINT
		descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;

		// MSAA를 사용한다면 D3D11_DSV_DIMENSION_TEXTURE2DMS를 써야함
		descDSV.Texture2D.MipSlice = 0;
		descDSV.Flags = 0;
		hr = m_pD3DDevice->CreateDepthStencilView(
			m_pDepthStencil,
			&descDSV,
			&m_pDepthStencilView);

		if (FAILED(hr)) return false;

		m_pImmediateContext->OMSetRenderTargets(
			1,
			&m_pRenderTargetView,
			m_pDepthStencilView);

		return true;
	}

	bool MapEditer::CreateRenderState(D3D11_FILL_MODE fillMode, D3D11_CULL_MODE cullMode)
	{
		D3D11_RASTERIZER_DESC rasterizerDesc;
		ZeroMemory(&rasterizerDesc, sizeof(D3D11_RASTERIZER_DESC));
		rasterizerDesc.FillMode = fillMode;		// Fill 옵션
		rasterizerDesc.CullMode = cullMode;	// Culling 옵션
		rasterizerDesc.FrontCounterClockwise = false;	  // 앞/뒷면 로직 선택 CCW
														  // 반시계 방향을 앞면으로 할 것인가?
		auto hr = m_pD3DDevice->CreateRasterizerState(
			&rasterizerDesc,
			&m_pSolidRS);

		if (FAILED(hr)) return false;
		return true;
	}

	bool MapEditer::CalcProc(float deltaTime)
	{
		m_pInputLayer->Update();
		OnKeyboardInput(deltaTime);
		CalculateMatrix();
		return true;
	}

	void MapEditer::CalculateMatrixForBox(float deltaTime)
	{
		static float accTime = 0;

		accTime += deltaTime / (FLOAT)100;
		// 박스를 회전시키기 위한 연산. 위치, 크기를 변경하고자 한다면 SRT를 기억할 것.
		XMMATRIX mat = XMMatrixRotationY(accTime);
		mat *= XMMatrixRotationX(-accTime);
		m_World = mat; // 여기서 g_world는 박스에 대한 Matrix임.

		XMMATRIX wvp = m_World * m_View * m_Projection;

		ConstantBuffer cb;
		cb.wvp = XMMatrixTranspose(wvp);
		cb.world = XMMatrixTranspose(m_World);
		cb.lightDir = m_LightDirection;
		cb.lightColor = m_LightColor;

		m_pImmediateContext->UpdateSubresource(m_pConstantBuffer, 0, 0, &cb, 0, 0); // update data
		m_pImmediateContext->VSSetConstantBuffers(0, 1, &m_pConstantBuffer);// set conatant buffer.
		m_pImmediateContext->PSSetConstantBuffers(0, 1, &m_pConstantBuffer);

		//ID3DX11EffectMatrixVariable* pWvp = m_pFX->GetVariableByName("wvp")->AsMatrix();
		//pWvp->SetMatrix((float*)(&wvp));

		//ID3DX11EffectMatrixVariable* pWorld = m_pFX->GetConstantBufferByName("world")->AsMatrix();
		//pWorld->SetMatrix((float*)(&m_World));

		//ID3DX11EffectVectorVariable* pLightDir = m_pFX->GetConstantBufferByName("lightDir")->AsVector();
		//pLightDir->SetFloatVector((float*)&m_LightDirection);

		//ID3DX11EffectVectorVariable* pLightColor = m_pFX->GetVariableByName("lightColor")->AsVector();
		//pLightColor->SetFloatVector((float*)&m_LightColor);

		//// Set Texture
		//ID3DX11EffectShaderResourceVariable* pDiffuseMap = nullptr;
		//pDiffuseMap = m_pFX->GetVariableByName("texDiffuse")->AsShaderResource();
		//pDiffuseMap->SetResource(m_pTextureRV);

		//// 사용할 Technique
		//ID3DX11EffectTechnique* tech = nullptr;
		//tech = m_pFX->GetTechniqueByName("NormalTech");

		//// Rendering
		//D3DX11_TECHNIQUE_DESC techDesc;
		//tech->GetDesc(&techDesc);
		//for (UINT p = 0; p < techDesc.Passes; ++p)
		//{
		//	tech->GetPassByIndex(p)->Apply(0, m_pImmediateContext); // p 값은 Pass 

		//	m_pImmediateContext->DrawIndexed(36, 0, 0);
		//}
	}

	void MapEditer::CalculateMatrixForBox2(float deltaTime)
	{
		static float accTime = 0.f;

		accTime += deltaTime / 15;
		XMMATRIX scale = XMMatrixScaling(1.0f, 1.0f, 1.0f);   // scale
		XMMATRIX rotate = XMMatrixRotationZ(accTime);    // rotate
		float moveValue = 5.0f;// move position
		XMMATRIX position = XMMatrixTranslation(moveValue, 0.0f, 0.0f);
		m_World2 = scale * rotate * position;     // S * R * T

		XMMATRIX rotate2 = XMMatrixRotationY(-accTime);    // rotate
		m_World2 *= rotate2;

		XMMATRIX wvp = m_World2  *  m_View  *  m_Projection;

		ID3DX11EffectMatrixVariable* pWvp = m_pFX->GetVariableByName("wvp")->AsMatrix();
		pWvp->SetMatrix(reinterpret_cast<float*>(&wvp));

		ID3DX11EffectMatrixVariable* pWorld = m_pFX->GetVariableByName("world")->AsMatrix();
		pWorld->SetMatrix(reinterpret_cast<float*>(&m_World2));

		ID3DX11EffectVectorVariable* pLightDir = m_pFX->GetVariableByName("lightDir")->AsVector();
		pLightDir->SetFloatVector((float*)&m_LightDirection);

		ID3DX11EffectVectorVariable* pLightColor = m_pFX->GetVariableByName("lightColor")->AsVector();
		pLightColor->SetFloatVector((float*)&m_LightColor);

		ID3DX11EffectTechnique* pTech = nullptr;
		pTech = m_pFX->GetTechniqueByName("NormalTech");

		D3DX11_TECHNIQUE_DESC techDesc;
		pTech->GetDesc(&techDesc);
		for (UINT p = 0; p < techDesc.Passes; ++p)
		{
			pTech->GetPassByIndex(p)->Apply(0, m_pImmediateContext);

			m_pImmediateContext->DrawIndexed(36, 0, 0);
		}
	}

	void MapEditer::CalculateMatrixForHeightMap(float deltaTime)
	{
		XMMATRIX mat = XMMatrixRotationY(0.0f);
		m_World = mat;

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

	void MapEditer::CalculateMatrix()
	{
		m_View = m_pCamera->GetView();
		m_Projection = m_pCamera->GetProj();
	}

	void MapEditer::OnKeyboardInput(float deltaTime)
	{
		auto CameraMoveSpeed = m_pCamera->GetMoveSpeed();
		if (m_pInputLayer->IsKeyDown(VK_W))
		{
			m_pCamera->Walk(CameraMoveSpeed * deltaTime);
		}
		if (m_pInputLayer->IsKeyDown(VK_S))
		{
			m_pCamera->Walk(-CameraMoveSpeed * deltaTime);
		}
		if (m_pInputLayer->IsKeyDown(VK_A))
		{
			m_pCamera->Strafe(-CameraMoveSpeed * deltaTime);
		}
		if (m_pInputLayer->IsKeyDown(VK_D))
		{
			m_pCamera->Strafe(CameraMoveSpeed * deltaTime);
		}

		m_pCamera->UpdateViewMatrix();
	}

	void MapEditer::OnMouseDown(WPARAM btnState, int x, int y)
	{
		m_LastMousePos.x = x;
		m_LastMousePos.y = y;

		SetCapture(m_hWnd);
	}

	void MapEditer::OnMouseUp(WPARAM btnState, int x, int y)
	{
		ReleaseCapture();
	}

	void MapEditer::OnMouseMove(WPARAM btnState, int x, int y)
	{
		if ((btnState & MK_LBUTTON) != 0)
		{
			// Make each pixel correspond to a quarter of a degree.
			float dx = XMConvertToRadians(0.25f * static_cast<float>(x - m_LastMousePos.x));
			float dy = XMConvertToRadians(0.25f * static_cast<float>(y - m_LastMousePos.y));

			m_pCamera->Pitch(dy);
			m_pCamera->RotateY(dx);
		}

		m_LastMousePos.x = x;
		m_LastMousePos.y = y;
	}


	bool MapEditer::DrawProc(float deltaTime)
	{
		float clearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f };
		m_pImmediateContext->VSSetShader(m_pVertexShader, NULL, 0);
		m_pImmediateContext->PSSetShader(m_pPixelShader, NULL, 0);

		m_pImmediateContext->PSSetShaderResources(0, 1, &m_pTextureRV);
		m_pImmediateContext->PSSetSamplers(0, 1, &m_pSamplerLinear);

		m_pImmediateContext->RSSetState(m_pSolidRS);
		m_pImmediateContext->ClearRenderTargetView(
			m_pRenderTargetView,
			clearColor);
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
		m_pImmediateContext->IASetVertexBuffers(
			0,
			1,
			&m_pHeightMapVertexBuffer,
			&stride,
			&offset);

		m_pImmediateContext->IASetIndexBuffer(
			m_pHeightMapIndexBuffer,
			DXGI_FORMAT_R32_UINT,
			0);

		//CalculateMatrixForBox(deltaTime);
		//m_pImmediateContext->DrawIndexed(36, 0, 0);

		CalculateMatrixForHeightMap(deltaTime);
		m_pImmediateContext->DrawIndexed(m_GridIndexCount, 0, 0);

		// Render (백버퍼를 프론트버퍼로 그린다.)
		m_pSwapChain->Present(0, 0);
		return true;
	}

	void MapEditer::InitMatrix()
	{
		m_World = XMMatrixIdentity();
		m_pCamera->SetPosition(0.0f, 0.0f, -8.0f);
		m_pCamera->SetLens(XM_PIDIV2, 800.0f / (FLOAT)600.f, 0.3f, 1000.0f);
		m_pCamera->UpdateViewMatrix();

		m_View = m_pCamera->GetView();
		m_Projection = m_pCamera->GetProj();
	}

	void MapEditer::MakeInnerObjects()
	{
		m_pInputLayer = new InputLayer;
		m_pInputLayer->Initialize();

		m_pTimer = new MyTimer;
		m_pTimer->Init();

		m_pCamera = new Camera;
	}

	bool MapEditer::CreateConstantBuffer()
	{
		D3D11_BUFFER_DESC cbd;
		ZeroMemory(&cbd, sizeof(cbd));
		cbd.Usage = D3D11_USAGE_DEFAULT;
		cbd.ByteWidth = sizeof(ConstantBuffer);
		cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		cbd.CPUAccessFlags = 0;
		auto hr = m_pD3DDevice->CreateBuffer(&cbd, NULL, &m_pConstantBuffer);

		if (FAILED(hr)) return false;
		return true;
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
		sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;   // 선형 보간 밉 레벨 필터링.
		sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;   // U좌표 Address Mode
		sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;   // V좌표 Address Mode
		sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;  // W좌표 Address Mode
		sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER; // 샘플링 데이터 비교 안함
		sampDesc.MinLOD = 0;			// 최소 Mipmap Range
		sampDesc.MaxLOD = D3D11_FLOAT32_MAX;	// 최대 Mipmap Range

		hr = m_pD3DDevice->CreateSamplerState(&sampDesc, &m_pSamplerLinear); // SamplerState 생성
		if (FAILED(hr))	return false;

		return true;
	}

	bool MapEditer::BuildGeometryBuffers()
	{
		MeshData grid;
		GeometryGenerator geoGen;

		geoGen.CreateGrid(160.0f, 160.0f, 50, 50, grid);
		m_GridIndexCount = grid.Indices32.size();

		std::vector<MyVertex> vertices(grid.Vertices.size());
		for (size_t i = 0; i < grid.Vertices.size(); ++i)
		{
			XMFLOAT3 p = grid.Vertices[i].Position;

			p.y = GetHeight(p.x, p.z);

			vertices[i].pos = p;

			if (p.y < -10.0f)
			{
				vertices[i].color = XMFLOAT4(1.0f, 0.96f, 0.62f, 1.0f);
			}
			else if (p.y < 5.0f)
			{
				vertices[i].color = XMFLOAT4(0.48f, 0.77f, 0.46f, 1.0f);
			}
			else if (p.y < 12.0f)
			{
				vertices[i].color = XMFLOAT4(0.1f, 0.48f, 0.19f, 1.0f);
			}
			else if (p.y < 20.0f)
			{
				vertices[i].color = XMFLOAT4(0.45f, 0.39f, 0.34f, 1.0f);
			}
			else 
			{
				vertices[i].color = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
			}
		}

		D3D11_BUFFER_DESC vbd;
		ZeroMemory(&vbd, sizeof(vbd));
		vbd.Usage = D3D11_USAGE_IMMUTABLE;
		vbd.ByteWidth = sizeof(MyVertex) * grid.Vertices.size();
		vbd.Usage = D3D11_USAGE_DEFAULT;
		vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vbd.CPUAccessFlags = 0;
		vbd.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA vinitData;
		ZeroMemory(&vinitData, sizeof(vinitData));
		vinitData.pSysMem = &vertices[0];
		auto hr = m_pD3DDevice->CreateBuffer(
			&vbd,
			&vinitData,
			&m_pHeightMapVertexBuffer);

		if (FAILED(hr)) return false;

		D3D11_BUFFER_DESC ibd;
		ZeroMemory(&ibd, sizeof(ibd));
		ibd.ByteWidth = sizeof(UINT) * m_GridIndexCount;
		ibd.Usage = D3D11_USAGE_DEFAULT;
		ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		ibd.CPUAccessFlags = 0;
		ibd.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA iinitData;
		ZeroMemory(&iinitData, sizeof(iinitData));
		iinitData.pSysMem = &grid.Indices32[0];
		hr = m_pD3DDevice->CreateBuffer(
			&ibd,
			&iinitData,
			&m_pHeightMapIndexBuffer);

		if (FAILED(hr)) return false;

		return true;
	}

	float MapEditer::GetHeight(float x, float z) const
	{
		return 0.3f * (z * sinf(0.1f * x) + x * cosf(0.1f * z));
	}

	/*
		내가 관심있는 메시지에 대해서만 처리해주는 콜백 함수.
	*/
	LRESULT CALLBACK MapEditer::MessageHandler(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
	{
		switch (iMessage)
		{
		case WM_LBUTTONDOWN :
		case WM_MBUTTONDOWN :
		case WM_RBUTTONDOWN :
			OnMouseDown(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
			return 0;

		case WM_LBUTTONUP :
		case WM_MBUTTONUP :
		case WM_RBUTTONUP :
			OnMouseUp(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
			return 0;

		case WM_MOUSEMOVE :
			OnMouseMove(wParam, GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
			return 0;

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