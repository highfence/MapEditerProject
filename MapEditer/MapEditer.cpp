#define _XM_NO_INTRINSICS_
#define _USE_MATH_DEFINES
#include "stdafx.h"
#include <iostream>
#include <string>
#include <DirectXMath.h>
#include <DirectXCollision.h>
#include <cfloat>
#include <math.h>
#include "InputLayer.h"
#include "Definition.h"
#include "MyTimer.h"
#include "Camera.h"
#include "GeometryGenerator.h"
#include "resource.h"
#include "MapEditer.h"

static int map_width = 0;
static int map_height = 0;
static int vertex_width = 0;
static int vertex_height = 0;

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
		MapSetting();
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

	void MapEditer::MapSetting()
	{
		DialogBox(m_hInstance, MAKEINTRESOURCE(IDD_DIALOG1), m_hWnd, MapSettingProc);
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
					//if (!DrawProc(AccTime)) break;
					if (!DrawProcWithEffect(AccTime)) break;
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
		retval = retval && CreateEffectShader();
		retval = retval && BuildGeometryBuffers();
		//retval = retval && CreateShader();
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

	bool MapEditer::CreateEffectShader()
	{
		ID3DBlob * pErrorBlob = nullptr;
		ID3DBlob * pCompileBlob = nullptr;

		HRESULT hr = D3DX11CompileFromFile(
			L"MyEffectShader.fx", 0, 0,
			0, "fx_5_0",
			0, 0, 0,
			&pCompileBlob, &pErrorBlob, 0);

		if (FAILED(hr)) return false;

		hr = D3DX11CreateEffectFromMemory(
			pCompileBlob->GetBufferPointer(),
			pCompileBlob->GetBufferSize(),
			0, m_pD3DDevice, &m_pFX);

		if (FAILED(hr)) return false;

		pCompileBlob->Release();

		D3D11_INPUT_ELEMENT_DESC layout[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};

		ID3DX11EffectTechnique * pTech = nullptr;
		pTech = m_pFX->GetTechniqueByName("NormalTech");
		D3DX11_PASS_DESC passDesc;
		pTech->GetPassByIndex(0)->GetDesc(&passDesc);

		UINT numElements = ARRAYSIZE(layout);
		hr = m_pD3DDevice->CreateInputLayout(
			layout,
			numElements,
			passDesc.pIAInputSignature,
			passDesc.IAInputSignatureSize,
			&m_pVertexLayout);

		if (FAILED(hr)) return false;

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
		auto hr = m_pD3DDevice->CreateBuffer(
			&bd,
			&initData,
			&m_pVertexBuffer);

		if (FAILED(hr)) return false;

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

		// 피킹된 물체에 적용할 stencilState를 만들어 놓기.
		D3D11_DEPTH_STENCIL_DESC pickedStencilDesc;
		pickedStencilDesc.DepthEnable = true;
		pickedStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		pickedStencilDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
		pickedStencilDesc.StencilEnable = false;

		hr = m_pD3DDevice->CreateDepthStencilState(&pickedStencilDesc, &m_pPickedStencilState);

		if (FAILED(hr)) return false;

		return true;
	}

	bool MapEditer::CreateRenderState(
		D3D11_FILL_MODE fillMode,
		D3D11_CULL_MODE cullMode)
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

		/* Effect FrameWork를 사용하기 전 코드.
		ConstantBuffer cb;
		cb.wvp = XMMatrixTranspose(wvp);
		cb.world = XMMatrixTranspose(m_World);
		cb.lightDir = m_LightDirection;
		cb.lightColor = m_LightColor;

		m_pImmediateContext->UpdateSubresource(m_pConstantBuffer, 0, 0, &cb, 0, 0); // update data
		m_pImmediateContext->VSSetConstantBuffers(0, 1, &m_pConstantBuffer);// set constant buffer.
		m_pImmediateContext->PSSetConstantBuffers(0, 1, &m_pConstantBuffer);
		*/

		ID3DX11EffectMatrixVariable * pWvp = nullptr;
		pWvp = m_pFX->GetVariableByName("wvp")->AsMatrix();
		pWvp->SetMatrix((float*)(&wvp));

		ID3DX11EffectMatrixVariable * pWorld = nullptr;
		pWorld = m_pFX->GetVariableByName("world")->AsMatrix();
		pWorld->SetMatrix((float*)(&m_World));

		ID3DX11EffectVectorVariable * pLightDir = nullptr;
		pLightDir = m_pFX->GetVariableByName("lightDir")->AsVector();
		pLightDir->SetFloatVector((float*)&m_LightDirection);

		ID3DX11EffectVectorVariable * pLightColor = nullptr;
		pLightColor = m_pFX->GetVariableByName("lightColor")->AsVector();
		pLightColor->SetFloatVector((float*)&m_LightColor);

		// 텍스쳐 세팅.
		ID3DX11EffectShaderResourceVariable * pDiffuseMap = nullptr;
		pDiffuseMap = m_pFX->GetVariableByName("texDiffuse")->AsShaderResource();
		pDiffuseMap->SetResource(m_pTextureRV);

		// 사용할 Technique
		ID3DX11EffectTechnique * pTech = nullptr;
		pTech = m_pFX->GetTechniqueByName("NormalTech");

		// 렌더링
		D3DX11_TECHNIQUE_DESC techDesc;
		pTech->GetDesc(&techDesc);
		UINT stride = sizeof(MyVertex);
		UINT offset = 0;

		for (UINT p = 0; p < techDesc.Passes; ++p)
		{
			m_pImmediateContext->IASetVertexBuffers(0, 1, &m_pHeightMapVertexBuffer, &stride, &offset);
			m_pImmediateContext->IASetIndexBuffer(m_pHeightMapIndexBuffer, DXGI_FORMAT_R32_UINT, 0);
			
			if (m_IsDrawWireFrame)
			{
				pTech->GetPassByIndex(2)->Apply(0, m_pImmediateContext);
			}
			else
			{
				pTech->GetPassByIndex(0)->Apply(0, m_pImmediateContext);
			}
			m_pImmediateContext->DrawIndexed(m_MeshData->Indices32.size(), 0, 0);

			// Restore default
			m_pImmediateContext->RSSetState(0);

			// 피킹된 삼각형이 존재하는 경우
			if (m_PickedTriangle != -1)
			{
				m_pImmediateContext->OMSetDepthStencilState(m_pPickedStencilState, 0);
				pTech->GetPassByIndex(p)->Apply(0, m_pImmediateContext);
				m_pImmediateContext->DrawIndexed(3, 3 * m_PickedTriangle, 0);

				m_pImmediateContext->OMSetDepthStencilState(0, 0);
			}
		}

		m_pSwapChain->Present(0, 0);
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
		if (m_pInputLayer->IsKeyDown(VK_TAB))
		{
			if (m_IsDrawWireFrame) m_IsDrawWireFrame = false;
			else m_IsDrawWireFrame = true;
		}
		if (m_pInputLayer->IsKeyDown(VK_PRIOR))
		{
			GeometryHeightChange(VK_PRIOR);
		}
		if (m_pInputLayer->IsKeyDown(VK_NEXT))
		{
			GeometryHeightChange(VK_NEXT);
		}

		m_pCamera->UpdateViewMatrix();
	}

	void MapEditer::OnMouseDown(WPARAM btnState, int x, int y)
	{
		// 왼쪽 버튼 클릭시 처리.
		if ((btnState & MK_LBUTTON) != 0)
		{
			m_LastMousePos.x = x;
			m_LastMousePos.y = y;

			SetCapture(m_hWnd);
		}
		// 오른쪽 버튼 클릭시 처리.
		else if ((btnState & MK_RBUTTON) != 0)
		{
			Pick(x, y);
		}
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

	bool MapEditer::DrawProcWithEffect(float deltaTime)
	{
		const FLOAT clearColor[4] = { 0.75f, 0.75f, 0.75f, 1.0f };
		m_pImmediateContext->ClearRenderTargetView(m_pRenderTargetView, clearColor);
		m_pImmediateContext->ClearDepthStencilView(m_pDepthStencilView,	D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

		// Set Input Assembler 
		m_pImmediateContext->IASetInputLayout(m_pVertexLayout);
		m_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		CalculateMatrixForHeightMap(deltaTime);
		return true;
	}

	void MapEditer::InitMatrix()
	{
		m_World = XMMatrixIdentity();
		m_pCamera->SetPosition(0.0f, 0.0f, -8.0f);
		m_pCamera->SetLens(XM_PIDIV2, m_Width / (FLOAT)m_Height, 0.3f, 1000.0f);
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
		if (m_MeshData != nullptr)
		{
			delete m_MeshData;
			m_MeshData = nullptr;
		}
		m_MeshData = new MeshData();

		GeometryGenerator geoGen;

		//geoGen.CreateGrid(150.0f, 150.0f, 20, 20, *m_MeshData);
		geoGen.CreateGrid(map_width, map_height, vertex_width, vertex_height, *m_MeshData);
		m_GridIndexCount = m_MeshData->Indices32.size();

		m_MeshData->Vertices.reserve(m_MeshData->Vertices.size());
		for (size_t i = 0; i < m_MeshData->Vertices.size(); ++i)
		{
			XMFLOAT3 p = m_MeshData->Vertices[i].pos;

			//p.y = GetHeight(p.x, p.z);

			m_MeshData->Vertices[i].pos = p;

			if (p.y < -10.0f)
			{
				m_MeshData->Vertices[i].color = XMFLOAT4(1.0f, 0.96f, 0.62f, 1.0f);
			}
			else if (p.y < 5.0f)
			{
				m_MeshData->Vertices[i].color = XMFLOAT4(0.48f, 0.77f, 0.46f, 1.0f);
			}
			else if (p.y < 12.0f)
			{
				m_MeshData->Vertices[i].color = XMFLOAT4(0.1f, 0.48f, 0.19f, 1.0f);
			}
			else if (p.y < 20.0f)
			{
				m_MeshData->Vertices[i].color = XMFLOAT4(0.45f, 0.39f, 0.34f, 1.0f);
			}
			else 
			{
				m_MeshData->Vertices[i].color = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
			}
		}

		D3D11_BUFFER_DESC vbd;
		ZeroMemory(&vbd, sizeof(vbd));
		vbd.Usage = D3D11_USAGE_DYNAMIC;
		vbd.ByteWidth = sizeof(MyVertex) * m_MeshData->Vertices.size();
		vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		vbd.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA vinitData;
		ZeroMemory(&vinitData, sizeof(vinitData));
		vinitData.pSysMem = &m_MeshData->Vertices[0];
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
		iinitData.pSysMem = &m_MeshData->Indices32[0];
		hr = m_pD3DDevice->CreateBuffer(
			&ibd,
			&iinitData,
			&m_pHeightMapIndexBuffer);

		if (FAILED(hr)) return false;

		return true;
	}

	float MapEditer::GetHeight(float x, float z) const
	{
		return 0.3f * (z * sinf(0.05f * x) + x * cosf(0.02f * z));
	}

	void MapEditer::Pick(int sx, int sy)
	{
		TranslateMousePosTrayWnd(&sx, &sy);
		// 메쉬 데이터가 아직 만들어지지 않은 상태라면, 에러로 판단.
		if (m_MeshData == nullptr) return;

		// 시야 공간에서의 피킹 반직선 계산.
		XMFLOAT4X4 P = m_pCamera->GetProj4x4f();

		float vx = (+2.0f * sx / m_Width - 1.0f) / P(0, 0);
		float vy = (-2.0f * sy / m_Height + 1.0f) / P(1, 1);

		// 반직선 정보 정의 (Origin은 시야공간에서 원점이다)
		XMVECTOR rayOrigin = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
		XMVECTOR rayDir = XMVectorSet(vx, vy, 1.0f, 0.0f);

		// 시야 공간에서 월드 공간으로 전환하는 행렬 구하기.
		XMMATRIX V = m_pCamera->GetView();
		XMMATRIX invView = XMMatrixInverse(&XMMatrixDeterminant(V), V);

		// 월드 공간에서 Mesh 공간으로 전환하는 행렬 구하기. 
		// (현재상태에서는 Identity Matrix이므로 별로 의미는 없음)
		XMMATRIX W = m_World;
		XMMATRIX invWorld = XMMatrixInverse(&XMMatrixDeterminant(W), W);

		// 시야 공간에서 Mesh의 로컬 공간으로 가는 행렬 정의
		XMMATRIX toLocal = XMMatrixMultiply(invView, invWorld);

		// 반직선 정보 갱신 (Coord는 점으로 처리, Normal은 벡터 처리)
		rayOrigin = XMVector3TransformCoord(rayOrigin, toLocal);
		rayDir = XMVector3TransformNormal(rayDir, toLocal);

		// 교차 판정을 위해 ray를 정규화.
		rayDir = XMVector3Normalize(rayDir);

		/* 교차 판정 시작. */
		// 선택 삼각형을 -1로 초기화한다.
		m_PickedTriangle = -1;
		// 임시로 가장 먼 거리를 잡은 tmin
		float tmin = 10000.0f;

		auto meshIndices = m_MeshData->Indices32;
		auto meshVertices = m_MeshData->Vertices;
		// 이 삼각형의 index를 쭉 찾아가면서 교차했는지 판정.
		for (UINT i = 0; i < meshIndices.size() / 3; ++i)
		{
			// 삼각형을 구축하는 인덱스.
			UINT i0 = meshIndices[i*3 + 0];
			UINT i1 = meshIndices[i*3 + 1];
			UINT i2 = meshIndices[i*3 + 2];

			// 인덱스에 따라 찾은 정점 정보.
			XMVECTOR v0 = XMLoadFloat3(&meshVertices[i0].pos);
			XMVECTOR v1 = XMLoadFloat3(&meshVertices[i1].pos);
			XMVECTOR v2 = XMLoadFloat3(&meshVertices[i2].pos);

			float t = 0.0f;
			if (DirectX::TriangleTests::Intersects(rayOrigin, rayDir, v0, v1, v2, t))
			{
				// 이 삼각형이 현재까지 가장 가까운 삼각형인지 판단.
				if (t <= tmin)
				{
					// 피킹된 삼각형의 정보를 갱신.
					tmin = t;
					m_PickedTriangle = i;
				}
			}
		}
	}

	void MapEditer::TranslateMousePosTrayWnd(int * sx, int * sy)
	{
		HWND hWndTrayWnd = ::FindWindow(TEXT("Shell_TrayWnd"), NULL);
		RECT rc;
		if (hWndTrayWnd)
		{
			GetWindowRect(hWndTrayWnd, &rc);
		}
	}

	const float changeDelta = 0.3f;
	void MapEditer::GeometryHeightChange(int inputKey)
	{
#pragma region util
		
		// 변한 데이터를 동적으로 매핑해줌.
		auto DataMapping = [this]()
		{
			D3D11_MAPPED_SUBRESOURCE mappedData;
			auto hr = m_pImmediateContext->Map(m_pHeightMapVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData);
			MyVertex* v = reinterpret_cast<MyVertex*>(mappedData.pData);

			for (UINT i = 0; i < m_MeshData->Vertices.size(); ++i)
			{
				v[i].pos = m_MeshData->Vertices[i].pos;
				v[i].color = m_MeshData->Vertices[i].color;
			}

			m_pImmediateContext->Unmap(m_pHeightMapIndexBuffer, 0);
		};

		// 거리에 따른 사인 그래프 높이 값을 반환해줌.
		auto GetSinHeight = [&](float stndHeight, float distance) -> float
		{
			if (m_SelectRange == 0.f) return FLT_MAX;
			auto normDist = distance / m_SelectRange;

			// 거리가 변화 범위보다 멀면 floatMax를 반환.
			if (normDist > 1.0f) return FLT_MAX;

			float sinInput = (float)1.57079632679489661923 + (float)1.57079632679489661923 * normDist;
			
			return stndHeight * sinf(sinInput);
		};

		// 좌표에 따른 거리 값을 반환해줌.
		auto GetDistance = [](float x1, float z1, float x2, float z2) -> float
		{
			auto dx = x1 - x2;
			auto dz = z1 - z2;

			auto dist = sqrtf(dx * dx + dz * dz);

			return dist;
		};

#pragma endregion

		// 선택한 삼각형이 없는 상태라면 바로 리턴.
		if (m_PickedTriangle == -1)
		{
			return;
		}

		auto& indices = m_MeshData->Indices32;
		auto& vertices = m_MeshData->Vertices;

		// 선택한 삼각형이 있는 상태라면, 인덱스를 가지고 Vertex를 역추적.
		auto& pickedVertexIdx = indices[m_PickedTriangle * 3];
		auto& pickedVertex = vertices[pickedVertexIdx];

		// Vertex의 높이를 조정.
		if (inputKey == VK_PRIOR)
		{
			// PAGE_UP이 눌린 경우, 높이를 올려줌.
			pickedVertex.pos.y += changeDelta;
			pickedVertex.color = GetColorByHeight(pickedVertex.pos.y);
		}
		else if (inputKey == VK_NEXT)
		{
			// PAGE_DOWN이 눌린 경우, 높이를 내려줌.
			pickedVertex.pos.y -= changeDelta;
			pickedVertex.color = GetColorByHeight(pickedVertex.pos.y);
		}

		// 역추적한 Vertex에서 가까이 있는 Vertex에 접근
		auto width = m_MeshData->GetWidthNum();
		auto height = m_MeshData->GetHeightNum();

		auto vec = GetSelectRange();
		for (auto& vertexIdx : vec)
		{
			auto dist = GetDistance(
				pickedVertex.pos.x,
				pickedVertex.pos.z,
				vertices[vertexIdx].pos.x,
				vertices[vertexIdx].pos.z);

			auto newHeight = GetSinHeight(pickedVertex.pos.y, dist);

			if (inputKey == VK_PRIOR)
			{
				if (newHeight > vertices[vertexIdx].pos.y)
				{
					vertices[vertexIdx].pos.y = newHeight;
					vertices[vertexIdx].color = GetColorByHeight(newHeight);
				}
			}
			else
			{ 
				if (newHeight < vertices[vertexIdx].pos.y)
				{
					vertices[vertexIdx].pos.y = newHeight;
					vertices[vertexIdx].color = GetColorByHeight(newHeight);
				}
			}
		}

		DataMapping();
	}

	// Picking된 점이 있을 때, 그 근처의 지점을 반환해주는 함수.
	std::vector<int> MapEditer::GetSelectRange()
	{
#pragma region Util

		// 좌표에 따른 거리 값을 반환해줌.
		auto GetDistance = [](float x1, float z1, float x2, float z2) -> float
		{
			auto dx = x1 - x2;
			auto dz = z1 - z2;

			auto dist = sqrtf(dx * dx + dz * dz);

			return dist;
		};

#pragma endregion
		std::vector<int> vertexVector;
		if (m_PickedTriangle == -1) return vertexVector;

		auto pickedVertexIdx = m_MeshData->Indices32[m_PickedTriangle * 3];
		auto pickedVertex = m_MeshData->Vertices[pickedVertexIdx];

		auto width = m_MeshData->GetWidthNum();
		auto height = m_MeshData->GetHeightNum();

		{
			//int curIdx = pickedVertexIdx;
			//// 위쪽으로
			//while (true)
			//{
			//	curIdx -= width;
			//	if (curIdx < 0) break;

			//	auto dist = GetDistance(
			//		pickedVertex.pos.x,
			//		pickedVertex.pos.z,
			//		m_MeshData->Vertices[curIdx].pos.x,
			//		m_MeshData->Vertices[curIdx].pos.z);

			//	if (dist > m_SelectRange) break;
			//	else vertexVector.push_back(curIdx);
			//}

			//// 왼쪽으로
			//curIdx = pickedVertexIdx;
			//int minIdx = pickedVertexIdx - pickedVertexIdx % width;
			//while (true)
			//{
			//	--curIdx;
			//	if (curIdx < minIdx) break;

			//	auto dist = GetDistance(
			//		pickedVertex.pos.x,
			//		pickedVertex.pos.z,
			//		m_MeshData->Vertices[curIdx].pos.x,
			//		m_MeshData->Vertices[curIdx].pos.z);

			//	if (dist > m_SelectRange) break;
			//	else vertexVector.push_back(curIdx);
			//}

			//// 오른쪽으로
			//curIdx = pickedVertexIdx;
			//int maxIdx = minIdx + width;
			//while (true)
			//{
			//	++curIdx;
			//	if (curIdx > maxIdx) break;

			//	auto dist = GetDistance(
			//		pickedVertex.pos.x,
			//		pickedVertex.pos.z,
			//		m_MeshData->Vertices[curIdx].pos.x,
			//		m_MeshData->Vertices[curIdx].pos.z);

			//	if (dist > m_SelectRange) break;
			//	else vertexVector.push_back(curIdx);
			//}

			//// 아래쪽으로
			//curIdx = pickedVertexIdx;
			//while (true)
			//{
			//	curIdx += width;
			//	if (curIdx >= m_MeshData->Vertices.size()) break;

			//	auto dist = GetDistance(
			//		pickedVertex.pos.x,
			//		pickedVertex.pos.z,
			//		m_MeshData->Vertices[curIdx].pos.x,
			//		m_MeshData->Vertices[curIdx].pos.z);

			//	if (dist > m_SelectRange) break;
			//	else vertexVector.push_back(curIdx);
			//}
		}

		for (int i = 0; i < m_MeshData->Vertices.size(); ++i)
		{
			auto dist = GetDistance(
					pickedVertex.pos.x,
					pickedVertex.pos.z,
					m_MeshData->Vertices[i].pos.x,
					m_MeshData->Vertices[i].pos.z); 

			if (dist < m_SelectRange)
			{
				vertexVector.push_back(i);
			}
		}

		return vertexVector;
	}

	XMFLOAT4 MapEditer::GetColorByHeight(float height)
	{
		if (height < -10.0f)
		{
			return XMFLOAT4(1.0f, 0.96f, 0.62f, 1.0f);
		}
		else if (height < 5.0f)
		{
			return XMFLOAT4(0.48f, 0.77f, 0.46f, 1.0f);
		}
		else if (height < 12.0f)
		{
			return XMFLOAT4(0.1f, 0.48f, 0.19f, 1.0f);
		}
		else if (height < 20.0f)
		{
			return XMFLOAT4(0.45f, 0.39f, 0.34f, 1.0f);
		}
		else
		{
			return XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
		}
	}



	/*
		내가 관심있는 메시지에 대해서만 처리해주는 콜백 함수.
	*/
	LRESULT CALLBACK MapEditer::MessageHandler(
		HWND hWnd,
		UINT iMessage,
		WPARAM wParam,
		LPARAM lParam)
	{
		switch (iMessage)
		{
		case WM_LBUTTONDOWN :
		case WM_MBUTTONDOWN :
		case WM_RBUTTONDOWN :
			OnMouseDown(
				wParam,
				GET_X_LPARAM(lParam),
				GET_Y_LPARAM(lParam));
			return 0;

		case WM_LBUTTONUP :
		case WM_MBUTTONUP :
			OnMouseUp(
				wParam,
				GET_X_LPARAM(lParam),
				GET_Y_LPARAM(lParam));
			return 0;
		case WM_RBUTTONUP :
			m_PickedTriangle = -1;
			return 0;

		case WM_MOUSEMOVE :
			OnMouseMove(
				wParam,
				GET_X_LPARAM(lParam),
				GET_Y_LPARAM(lParam));
			return 0;

		default:
			return (DefWindowProc(hWnd, iMessage, wParam, lParam));
		}
	}

	LRESULT CALLBACK WndProc(
		HWND hWnd,
		UINT iMessage,
		WPARAM wParam,
		LPARAM lParam)
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

	BOOL CALLBACK MapSettingProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
	{

		switch (iMessage)
		{
		case WM_INITDIALOG :
			SetDlgItemInt(hWnd, IDC_MAP_WIDTH, map_width, FALSE);
			SetDlgItemInt(hWnd, IDC_MAP_HEIGHT, map_height, FALSE);
			SetDlgItemInt(hWnd, IDC_VERTEX_WIDTH, vertex_width, FALSE);
			SetDlgItemInt(hWnd, IDC_VERTEX_HEIGHT, vertex_height, FALSE);
			return TRUE;

		case WM_COMMAND :
			switch (LOWORD(wParam))
			{
			case IDOK :
				map_width = GetDlgItemInt(hWnd, IDC_MAP_WIDTH, NULL, FALSE);
				map_height = GetDlgItemInt(hWnd, IDC_MAP_HEIGHT, NULL, FALSE);
				vertex_width = GetDlgItemInt(hWnd, IDC_VERTEX_WIDTH, NULL, FALSE);
				vertex_height = GetDlgItemInt(hWnd, IDC_VERTEX_HEIGHT, NULL, FALSE);
				EndDialog(hWnd, IDOK);
				return TRUE;

			case IDCANCEL :
				PostQuitMessage(0);
				return TRUE;
			}
		}
		return FALSE;
	}
}