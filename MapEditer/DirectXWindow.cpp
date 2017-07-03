#include <iostream>
#include <Windows.h>
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
#pragma region Create Func

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
			bool retval = true;
			retval = retval && CreateEffectShader();
			retval = retval && BuildGeometryBuffers();
			retval = retval && CreateConstantBuffer();
			retval = retval && CreateRenderState(
				D3D11_FILL_SOLID,
				D3D11_CULL_BACK);
		};

#pragma endregion 

		MakeWindow(hInst, hWnd);
		InitDirectX();

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

	LRESULT DirectXWindow::DirectXWindowProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
	{
		switch (iMessage)
		{
		

		}

		return (DefWindowProc(hWnd, iMessage, wParam, lParam));
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

		//geoGen.CreateGrid(150.0f, 150.0f, 20, 20, *m_MeshData);
		geoGen.CreateGrid(_MapWidth, _MapHeight, _GridWidth, _GridHeight, *m_MeshData);
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

	void DirectXWindow::CleanupDevice()
	{
		if (m_pFX)				 m_pFX->Release();
		if (m_pImmediateContext) m_pImmediateContext->ClearState();

		if (m_pSolidRS)          m_pSolidRS->Release();

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

}