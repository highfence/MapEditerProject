#include <iostream>
#include <Windows.h>
#include <windowsx.h>
#include <DirectXMath.h>
#include <DirectXCollision.h>

#include <cfloat>
#include <math.h>

#include <dxgi.h>
#include <d3d11.h>
#include <d3dCompiler.h>
#include <d3dx11.h>
#include <dxerr.h>
#include <d3dx11effect.h>

#pragma comment( lib, "dxguid.lib" )
#pragma comment( lib, "d3dcompiler.lib" )
#pragma comment( lib, "dxerr.lib" )
#pragma comment( lib, "dxgi.lib" )
#pragma comment( lib, "d3d11.lib" )
#pragma comment( lib, "d3dx11d.lib" )
#pragma comment( lib, "Effects11d.lib" )

#include "InputLayer.h"
#include "Camera.h"
#include "Definition.h"
#include "GeometryGenerator.h"
#include "DirectXWindow.h"

namespace DXMapEditer
{
	DirectXWindow::~DirectXWindow()
	{
		CleanupDevice();
		DestroyWindow(_hThis);
	}

	void DirectXWindow::CreateDXWindow(HINSTANCE hInst, HWND hWnd)
	{
		_hWnd = hWnd;
#pragma region Create Func

		auto InnerClassInitialize = [this]()
		{
			m_pInputLayer = new InputLayer();
			m_pInputLayer->Initialize();

			m_pCamera = new Camera();
		};

		auto MakeWindow = [this](HINSTANCE hInst, HWND hWnd)
		{
			WNDCLASS WndClass;
			WndClass.cbClsExtra = 0;
			WndClass.cbWndExtra = 0;
			WndClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
			WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
			WndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
			WndClass.hInstance = hInst;
			WndClass.lpfnWndProc = DirectXWindowProc;
			WndClass.lpszClassName = L"MapEditer Window";
			WndClass.lpszMenuName = NULL;
			WndClass.style = CS_HREDRAW | CS_VREDRAW;

			if (!RegisterClass(&WndClass)) return;

			_hThis = CreateWindow(
				TEXT("MapEditer Window"), NULL, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN,
				0, 0, 0, 0, hWnd, (HMENU)0, hInst, NULL);
		};

		auto InitDirectX = [this]() -> bool
		{
			if (!CreateDeviceAndSwapChain()) return false;
			if (!CreateRenderTargetView()) return false;
			if (!CreateViewPort()) return false;
			if (!CreateDepthStencilTexture()) return false;

			return true;
		};

		auto DirectXSetting = [this]()
		{
			CreateEffectShader();
			BuildGeometryBuffers();
			CreateConstantBuffer();
			CreateRenderState(
				D3D11_FILL_SOLID,
				D3D11_CULL_BACK);
		};

		auto InitMatrix = [this]()
		{
			m_World = XMMatrixIdentity();
			m_pCamera->SetPosition(0.0f, 0.0f, -8.0f);
			m_pCamera->SetLens(XM_PIDIV2, _ClientWidth / (FLOAT)_ClientHeight, 0.3f, 1000.0f);
			m_pCamera->UpdateViewMatrix();

			m_View = m_pCamera->GetView();
			m_Projection = m_pCamera->GetProj();
		};

#pragma endregion 

		InnerClassInitialize();
		MakeWindow(hInst, hWnd);
		InitDirectX();
		InitMatrix();
		
		DirectXSetting();

		dxWindowHandler = this;
	}

	void DirectXWindow::MoveDXWindow()
	{
		MoveWindow(_hThis, 0, 0, 800, 600, TRUE);
	}

	void DirectXWindow::SetGridVariables(int mapWidth, int mapHeight, int gridWidth, int gridHeight)
	{
		_MapWidth = mapWidth;
		_MapHeight = mapHeight;
		_GridWidth = gridWidth;
		_GridHeight = gridHeight;
	}

	void DirectXWindow::CalcProc(const float deltaTime)
	{
		m_pInputLayer->Update();
		OnKeyboardInput(deltaTime);

		m_View = m_pCamera->GetView();
		m_Projection = m_pCamera->GetProj();
	}

	void DirectXWindow::DrawProc(const float deltaTime)
	{
		const FLOAT clearColor[4] = { 0.75f, 0.75f, 0.75f, 1.0f };
		m_pImmediateContext->ClearRenderTargetView(m_pRenderTargetView, clearColor);
		m_pImmediateContext->ClearDepthStencilView(m_pDepthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

		// Set Input Assembler 
		m_pImmediateContext->IASetInputLayout(m_pVertexLayout);
		m_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		CalculateMatrixForHeightMap(deltaTime);
		return;
	}

	LRESULT DirectXWindow::MessageHandler(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
	{
		switch (iMessage)
		{
		case WM_LBUTTONDOWN:
		case WM_MBUTTONDOWN:
		case WM_RBUTTONDOWN:
			OnMouseDown(
				wParam,
				GET_X_LPARAM(lParam),
				GET_Y_LPARAM(lParam));
			return 0;

		case WM_LBUTTONUP:
		case WM_MBUTTONUP:
			OnMouseUp(
				wParam,
				GET_X_LPARAM(lParam),
				GET_Y_LPARAM(lParam));
			return 0;
		case WM_RBUTTONUP:
			m_PickedTriangle = -1;
			return 0;

		case WM_MOUSEMOVE:
			OnMouseMove(
				wParam,
				GET_X_LPARAM(lParam),
				GET_Y_LPARAM(lParam));
			return 0;

		default:
			return (DefWindowProc(hWnd, iMessage, wParam, lParam));
		}
	}

	LRESULT DirectXWindowProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
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
			return dxWindowHandler->MessageHandler(hWnd, iMessage, wParam, lParam);
		}
	}

	bool DirectXWindow::CreateDeviceAndSwapChain()
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

		sd.BufferDesc.Width = _ClientWidth;
		sd.BufferDesc.Height = _ClientHeight;
		sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		sd.BufferDesc.RefreshRate.Numerator = 60;
		sd.BufferDesc.RefreshRate.Denominator = 1;

		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sd.OutputWindow = _hThis;
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

	bool DirectXWindow::CreateRenderTargetView()
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

	bool DirectXWindow::CreateViewPort()
	{
		D3D11_VIEWPORT vp;
		vp.Width = (FLOAT)_ClientWidth;
		vp.Height = (FLOAT)_ClientHeight;
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
		vp.TopLeftX = 0;
		vp.TopLeftY = 0;
		m_pImmediateContext->RSSetViewports(1, &vp);
		return true;
	}

	bool DirectXWindow::CreateDepthStencilTexture()
	{
		// Create depth stencil texture
		D3D11_TEXTURE2D_DESC descDepth;
		ZeroMemory(&descDepth, sizeof(descDepth));
		descDepth.Width = _ClientWidth;
		descDepth.Height = _ClientHeight;
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

	}

	bool DirectXWindow::CreateEffectShader()
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
	}

	bool DirectXWindow::BuildGeometryBuffers()
	{
		if (m_MeshData != nullptr)
		{
			delete m_MeshData;
			m_MeshData = nullptr;
		}
		m_MeshData = new MeshData();

		GeometryGenerator geoGen;

		geoGen.CreateGrid(150.0f, 150.0f, 20, 20, *m_MeshData);
		//geoGen.CreateGrid(_MapWidth, _MapHeight, _GridWidth, _GridHeight, *m_MeshData);
		_GridIndexCount = m_MeshData->Indices32.size();

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
		ibd.ByteWidth = sizeof(UINT) * _GridIndexCount;
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

	bool DirectXWindow::CreateConstantBuffer()
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

	bool DirectXWindow::CreateRenderState(
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

	void DirectXWindow::LoadTexture()
	{
		// TODO :: 구지 필요 없을 것 같긴한데. 일단.
		auto hr = D3DX11CreateShaderResourceViewFromFile(
			m_pD3DDevice,
			L"./images.jpg",
			NULL,
			NULL,
			&m_pTextureRV,
			NULL);

		if (FAILED(hr)) return;

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
		if (FAILED(hr))	return;

		return;
	}

	void DirectXWindow::CleanupDevice()
	{
		if (m_pFX)				 m_pFX->Release();
		if (m_pImmediateContext) m_pImmediateContext->ClearState();
		if (m_pSamplerLinear)	 m_pSamplerLinear->Release();

		if (m_pSolidRS)          m_pSolidRS->Release();
		if (m_pTextureRV)		 m_pTextureRV->Release();

		if (m_pDepthStencilView) m_pDepthStencilView->Release();
		if (m_pConstantBuffer)   m_pConstantBuffer->Release();
		if (m_pHeightMapIndexBuffer) m_pHeightMapIndexBuffer->Release();
		if (m_pHeightMapVertexBuffer) m_pHeightMapVertexBuffer->Release();

		if (m_pVertexLayout)     m_pVertexLayout->Release();

		if (m_pRenderTargetView) m_pRenderTargetView->Release();
		if (m_pSwapChain)        m_pSwapChain->Release();
		if (m_pImmediateContext) m_pImmediateContext->Release();
		if (m_pD3DDevice)        m_pD3DDevice->Release();
	}

	void DirectXWindow::CalculateMatrixForHeightMap(const float deltaTime)
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

	const float changeDelta = 0.3f;
	void DirectXWindow::GeometryHeightChange(int inputKey)
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

	XMFLOAT4 DirectXWindow::GetColorByHeight(float height)
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

	std::vector<int> DirectXWindow::GetSelectRange()
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

	void DirectXWindow::Pick(int sx, int sy)
	{
		// 메쉬 데이터가 아직 만들어지지 않은 상태라면, 에러로 판단.
		if (m_MeshData == nullptr) return;

		// 시야 공간에서의 피킹 반직선 계산.
		XMFLOAT4X4 P = m_pCamera->GetProj4x4f();

		float vx = (+2.0f * sx / _ClientWidth - 1.0f) / P(0, 0);
		float vy = (-2.0f * sy / _ClientHeight + 1.0f) / P(1, 1);

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
			UINT i0 = meshIndices[i * 3 + 0];
			UINT i1 = meshIndices[i * 3 + 1];
			UINT i2 = meshIndices[i * 3 + 2];

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

	void DirectXWindow::OnKeyboardInput(float deltaTime)
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

	void DirectXWindow::OnMouseDown(WPARAM btnState, int x, int y)
	{
		// 왼쪽 버튼 클릭시 처리.
		if ((btnState & MK_LBUTTON) != 0)
		{
			m_LastMousePos.x = x;
			m_LastMousePos.y = y;

			SetCapture(_hThis);
		}
		// 오른쪽 버튼 클릭시 처리.
		else if ((btnState & MK_RBUTTON) != 0)
		{
			Pick(x, y);
		}
	}

	void DirectXWindow::OnMouseUp(WPARAM btnState, int x, int y)
	{
		ReleaseCapture();
	}

	void DirectXWindow::OnMouseMove(WPARAM btnState, int x, int y)
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

}