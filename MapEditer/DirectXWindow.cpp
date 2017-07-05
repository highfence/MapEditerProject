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
		cleanupDevice();
		DestroyWindow(_hThis);
	}

	void DirectXWindow::CreateDXWindow(HINSTANCE hInst, HWND hWnd)
	{
		_hWnd = hWnd;
#pragma region Create Func

		auto InnerClassInitialize = [this]()
		{
			_inputLayer = new InputLayer();
			_inputLayer->Initialize();

			_camera = new Camera();
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
			if (!createDeviceAndSwapChain()) return false;
			if (!createRenderTargetView()) return false;
			if (!createViewPort()) return false;
			if (!createDepthStencilTexture()) return false;

			return true;
		};

		auto DirectXSetting = [this]()
		{
			createEffectShader();
			buildGeometryBuffers();
			createConstantBuffer();
			createRenderState(
				D3D11_FILL_SOLID,
				D3D11_CULL_BACK);
			loadTexture();
		};

		auto InitMatrix = [this]()
		{
			_world = XMMatrixIdentity();
			_camera->SetPosition(0.0f, 50.0f, -8.0f);
			_camera->SetLens(XM_PIDIV2, _clientWidth / (FLOAT)_clientHeight, 0.3f, 1000.0f);
			_camera->UpdateViewMatrix();

			_view = _camera->GetView();
			_projection = _camera->GetProj();
		};


#pragma endregion 

		InnerClassInitialize();
		MakeWindow(hInst, hWnd);
		InitDirectX();
		InitMatrix();
		checkDrawEnabled();
		
		DirectXSetting();

		dxWindowHandler = this;
	}

	void DirectXWindow::MoveDXWindow()
	{
		MoveWindow(_hThis, 0, 0, 800, 600, TRUE);
	}

	void DirectXWindow::SetGridVariables(int mapWidth, int mapHeight, int gridWidth, int gridHeight)
	{
		_mapWidth = mapWidth;
		_mapHeight = mapHeight;
		_gridWidth = gridWidth;
		_gridHeight = gridHeight;
	}

	void DirectXWindow::CalcProc(const float deltaTime)
	{
		if (_isLeftMouseDown = true &&
			_pickingType != (int)OPT_WINDOW_FUNCTIONS::PICKING_MOVE_SELECTED &&
			_pickedTriangle != -1)
		{
			geometryHeightChange();
		}

		_inputLayer->Update();
		onKeyboardInput(deltaTime);

		_view = _camera->GetView();
		_projection = _camera->GetProj();

		dataMapping();
	}

	void DirectXWindow::DrawProc(const float deltaTime)
	{
		const FLOAT clearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f };
		_immediateContext->ClearRenderTargetView(_renderTargetView, clearColor);
		_immediateContext->ClearDepthStencilView(_depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);

		// Set Input Assembler 
		_immediateContext->IASetInputLayout(_vertexLayout);
		_immediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);


		// Texture
		{
			_immediateContext->PSSetShaderResources(0, 1, &_textureRV);
			_immediateContext->PSSetSamplers(0, 1, &_samplerLinear);
		}

		if (_isDrawEnabled)
		{
			calculateMatrixForHeightMap(deltaTime);
		}
		return;
	}

	LRESULT DirectXWindow::MessageHandler(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
	{
		switch (iMessage)
		{
		case WM_LBUTTONDOWN:
		case WM_MBUTTONDOWN:
		case WM_RBUTTONDOWN:
			onMouseDown(
				wParam,
				GET_X_LPARAM(lParam),
				GET_Y_LPARAM(lParam));
			return 0;

		case WM_LBUTTONUP:
		case WM_MBUTTONUP:
			onMouseUp(
				wParam,
				GET_X_LPARAM(lParam),
				GET_Y_LPARAM(lParam));
			return 0;
		case WM_RBUTTONUP:
			_pickedTriangle = -1;
			return 0;

		case WM_MOUSEMOVE:
			onMouseMove(
				wParam,
				GET_X_LPARAM(lParam),
				GET_Y_LPARAM(lParam));
			return 0;

		default:
			return (DefWindowProc(hWnd, iMessage, wParam, lParam));
		}
	}

	void DirectXWindow::SetCameraMove(int cameraMoveSpeed)
	{
		_camera->SetMoveSpeed(cameraMoveSpeed);
	}

	void DirectXWindow::SetPickingType(int pickedButton)
	{
		// 들어온 값이 pickedType이 아니라면 그냥 반환. 
		if (pickedButton < (int)OPT_WINDOW_FUNCTIONS::PICKING_MOVE_SELECTED
			|| pickedButton > (int)OPT_WINDOW_FUNCTIONS::PICKING_STND_SELECTED)
		{
			return;
		}

		_pickingType = pickedButton;
	}

	void DirectXWindow::SetSelectRange(int range)
	{
		_pickingRange = range;
	}

	void DirectXWindow::GridInitialize(int initFlag)
	{
		_meshData->Initialize();
	}

	void DirectXWindow::CheckoutWireframe(int flag)
	{
		if (_isDrawWireFrame)
		{
			_isDrawWireFrame = false;
		}
		else
		{
			_isDrawWireFrame = true;
		}
	}

	void DirectXWindow::GoCameraToOrigin(int flag)
	{
		_camera->GoOrigin(_clientWidth, _clientHeight);
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

	bool DirectXWindow::createDeviceAndSwapChain()
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

		sd.BufferDesc.Width = _clientWidth;
		sd.BufferDesc.Height = _clientHeight;
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
			&_swapChain,
			&_d3dDevice,
			&_featureLevel,
			&_immediateContext);

		if (FAILED(hr)) return false;

		return true;
	}

	bool DirectXWindow::createRenderTargetView()
	{
		ID3D11Texture2D* pBackBuffer = NULL;
		auto hr = _swapChain->GetBuffer(0,
			__uuidof(ID3D11Texture2D),
			(LPVOID*)&pBackBuffer);

		if (FAILED(hr)) return false;

		hr = _d3dDevice->CreateRenderTargetView(
			pBackBuffer,
			NULL,
			&_renderTargetView);

		pBackBuffer->Release();

		if (FAILED(hr)) return false;

		_immediateContext->OMSetRenderTargets(
			1,
			&_renderTargetView,
			NULL);

		return true;
	}

	bool DirectXWindow::createViewPort()
	{
		D3D11_VIEWPORT vp;
		vp.Width = (FLOAT)_clientWidth;
		vp.Height = (FLOAT)_clientHeight;
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
		vp.TopLeftX = 0;
		vp.TopLeftY = 0;
		_immediateContext->RSSetViewports(1, &vp);
		return true;
	}

	bool DirectXWindow::createDepthStencilTexture()
	{
		// Create depth stencil texture
		D3D11_TEXTURE2D_DESC descDepth;
		ZeroMemory(&descDepth, sizeof(descDepth));
		descDepth.Width = _clientWidth;
		descDepth.Height = _clientHeight;
		descDepth.MipLevels = 1;
		descDepth.ArraySize = 1;
		descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		descDepth.SampleDesc.Count = 1;
		descDepth.SampleDesc.Quality = 0;
		descDepth.Usage = D3D11_USAGE_DEFAULT;
		descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		descDepth.CPUAccessFlags = 0;
		descDepth.MiscFlags = 0;
		auto hr = _d3dDevice->CreateTexture2D(&descDepth, NULL, &_depthStencil);

		if (FAILED(hr)) return false;

		// Create the depth stencil view
		D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
		ZeroMemory(&descDSV, sizeof(descDSV));
		descDSV.Format = descDepth.Format; // == DXGI_FORMAT_D24_UNORM_S8_UINT
		descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;

		// MSAA를 사용한다면 D3D11_DSV_DIMENSION_TEXTURE2DMS를 써야함
		descDSV.Texture2D.MipSlice = 0;
		descDSV.Flags = 0;
		hr = _d3dDevice->CreateDepthStencilView(
			_depthStencil,
			&descDSV,
			&_depthStencilView);

		if (FAILED(hr)) return false;

		_immediateContext->OMSetRenderTargets(
			1,
			&_renderTargetView,
			_depthStencilView);

		// 피킹된 물체에 적용할 stencilState를 만들어 놓기.
		D3D11_DEPTH_STENCIL_DESC pickedStencilDesc;
		pickedStencilDesc.DepthEnable = true;
		pickedStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		pickedStencilDesc.DepthFunc = D3D11_COMPARISON_LESS_EQUAL;
		pickedStencilDesc.StencilEnable = false;

		hr = _d3dDevice->CreateDepthStencilState(&pickedStencilDesc, &_pickedStencilState);

		if (FAILED(hr)) return false;

	}

	bool DirectXWindow::createEffectShader()
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
			0, _d3dDevice, &_fx);

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
		pTech = _fx->GetTechniqueByName("NormalTech");
		D3DX11_PASS_DESC passDesc;
		pTech->GetPassByIndex(0)->GetDesc(&passDesc);

		UINT numElements = ARRAYSIZE(layout);
		hr = _d3dDevice->CreateInputLayout(
			layout,
			numElements,
			passDesc.pIAInputSignature,
			passDesc.IAInputSignatureSize,
			&_vertexLayout);

		if (FAILED(hr)) return false;
		return true;
	}

	bool DirectXWindow::buildGeometryBuffers()
	{
		if (!_isDrawEnabled) return false;
		if (_meshData != nullptr)
		{
			delete _meshData;
			_meshData = nullptr;
		}

		_meshData = new MeshData();

		GeometryGenerator geoGen;

		geoGen.CreateGrid(_mapWidth, _mapHeight, _gridWidth, _gridHeight, *_meshData);
		_gridIndexCount = _meshData->Indices32.size();

		_meshData->Vertices.reserve(_meshData->Vertices.size());
		for (size_t i = 0; i < _meshData->Vertices.size(); ++i)
		{
			XMFLOAT3 p = _meshData->Vertices[i].pos;

			//p.y = GetHeight(p.x, p.z);

			_meshData->Vertices[i].pos = p;

			if (p.y < -10.0f)
			{
				_meshData->Vertices[i].color = XMFLOAT4(1.0f, 0.96f, 0.62f, 1.0f);
			}
			else if (p.y < 5.0f)
			{
				_meshData->Vertices[i].color = XMFLOAT4(0.48f, 0.77f, 0.46f, 1.0f);
			}
			else if (p.y < 12.0f)
			{
				_meshData->Vertices[i].color = XMFLOAT4(0.1f, 0.48f, 0.19f, 1.0f);
			}
			else if (p.y < 20.0f)
			{
				_meshData->Vertices[i].color = XMFLOAT4(0.45f, 0.39f, 0.34f, 1.0f);
			}
			else
			{
				_meshData->Vertices[i].color = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
			}
		}

		D3D11_BUFFER_DESC vbd;
		ZeroMemory(&vbd, sizeof(vbd));
		vbd.Usage = D3D11_USAGE_DYNAMIC;
		vbd.ByteWidth = sizeof(MyVertex) * _meshData->Vertices.size();
		vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		vbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		//vbd.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA vinitData;
		ZeroMemory(&vinitData, sizeof(vinitData));
		vinitData.pSysMem = &_meshData->Vertices[0];
		auto hr = _d3dDevice->CreateBuffer(
			&vbd,
			&vinitData,
			&_heightMapVertexBuffer);

		if (FAILED(hr)) return false;

		D3D11_BUFFER_DESC ibd;
		ZeroMemory(&ibd, sizeof(ibd));
		ibd.ByteWidth = sizeof(UINT) * _gridIndexCount;
		ibd.Usage = D3D11_USAGE_DEFAULT;
		ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		ibd.CPUAccessFlags = 0;
		ibd.MiscFlags = 0;

		D3D11_SUBRESOURCE_DATA iinitData;
		ZeroMemory(&iinitData, sizeof(iinitData));
		iinitData.pSysMem = &_meshData->Indices32[0];
		hr = _d3dDevice->CreateBuffer(
			&ibd,
			&iinitData,
			&_heightMapIndexBuffer);

		if (FAILED(hr)) return false;

		return true;
	}

	bool DirectXWindow::createConstantBuffer()
	{
		D3D11_BUFFER_DESC cbd;
		ZeroMemory(&cbd, sizeof(cbd));
		cbd.Usage = D3D11_USAGE_DEFAULT;
		cbd.ByteWidth = sizeof(ConstantBuffer);
		cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		cbd.CPUAccessFlags = 0;
		auto hr = _d3dDevice->CreateBuffer(&cbd, NULL, &_constantBuffer);

		if (FAILED(hr)) return false;
		return true;
	}

	bool DirectXWindow::createRenderState(
		D3D11_FILL_MODE fillMode,
		D3D11_CULL_MODE cullMode)
	{
		D3D11_RASTERIZER_DESC rasterizerDesc;
		ZeroMemory(&rasterizerDesc, sizeof(D3D11_RASTERIZER_DESC));
		rasterizerDesc.FillMode = fillMode;		// Fill 옵션
		rasterizerDesc.CullMode = cullMode;	// Culling 옵션
		rasterizerDesc.FrontCounterClockwise = false;	  // 앞/뒷면 로직 선택 CCW
														  // 반시계 방향을 앞면으로 할 것인가?
		auto hr = _d3dDevice->CreateRasterizerState(
			&rasterizerDesc,
			&_solidRS);

		if (FAILED(hr)) return false;
		return true;
	}

	void DirectXWindow::loadTexture()
	{
		auto hr = D3DX11CreateShaderResourceViewFromFile(
			_d3dDevice,
			L"./images.jpg",
			NULL,
			NULL,
			&_textureRV,
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

		hr = _d3dDevice->CreateSamplerState(&sampDesc, &_samplerLinear); // SamplerState 생성
		if (FAILED(hr))	return;

		return;
	}

	void DirectXWindow::checkDrawEnabled()
	{
		// Grid 구성 요소중 하나만이라도 0이면, 
		if (!_mapWidth || !_mapHeight || !_gridWidth || !_gridHeight)
		{
			// 그릴 수 없다.
			_isDrawEnabled = false;
		}
		else
		{
			_isDrawEnabled = true;
		}
	}

	void DirectXWindow::cleanupDevice()
	{
		if (_fx)				 _fx->Release();
		if (_immediateContext) _immediateContext->ClearState();
		if (_samplerLinear)	 _samplerLinear->Release();

		if (_solidRS)          _solidRS->Release();
		if (_textureRV)		 _textureRV->Release();

		if (_depthStencilView) _depthStencilView->Release();
		if (_constantBuffer)   _constantBuffer->Release();
		if (_heightMapIndexBuffer) _heightMapIndexBuffer->Release();
		if (_heightMapVertexBuffer) _heightMapVertexBuffer->Release();

		if (_vertexLayout)     _vertexLayout->Release();

		if (_renderTargetView) _renderTargetView->Release();
		if (_swapChain)        _swapChain->Release();
		if (_immediateContext) _immediateContext->Release();
		if (_d3dDevice)        _d3dDevice->Release();
	}

	void DirectXWindow::calculateMatrixForHeightMap(const float deltaTime)
	{
		XMMATRIX mat = XMMatrixRotationY(0.0f);
		_world = mat;

		XMMATRIX wvp = _world * _view * _projection;

		/* Effect FrameWork를 사용하기 전 코드.
		ConstantBuffer cb;
		cb.wvp = XMMatrixTranspose(wvp);
		cb.world = XMMatrixTranspose(_world);
		cb.lightDir = _lightDirection;
		cb.lightColor = _lightColor;

		_immediateContext->UpdateSubresource(_constantBuffer, 0, 0, &cb, 0, 0); // update data
		_immediateContext->VSSetConstantBuffers(0, 1, &_constantBuffer);// set constant buffer.
		_immediateContext->PSSetConstantBuffers(0, 1, &_constantBuffer);
		*/

		ID3DX11EffectMatrixVariable * pWvp = nullptr;
		pWvp = _fx->GetVariableByName("wvp")->AsMatrix();
		pWvp->SetMatrix((float*)(&wvp));

		ID3DX11EffectMatrixVariable * pWorld = nullptr;
		pWorld = _fx->GetVariableByName("world")->AsMatrix();
		pWorld->SetMatrix((float*)(&_world));

		ID3DX11EffectVectorVariable * pLightDir = nullptr;
		pLightDir = _fx->GetVariableByName("lightDir")->AsVector();
		pLightDir->SetFloatVector((float*)&_lightDirection);

		ID3DX11EffectVectorVariable * pLightColor = nullptr;
		pLightColor = _fx->GetVariableByName("lightColor")->AsVector();
		pLightColor->SetFloatVector((float*)&_lightColor);

		// 텍스쳐 세팅.
		ID3DX11EffectShaderResourceVariable * pDiffuseMap = nullptr;
		pDiffuseMap = _fx->GetVariableByName("texDiffuse")->AsShaderResource();
		pDiffuseMap->SetResource(_textureRV);

		// 사용할 Technique
		ID3DX11EffectTechnique * pTech = nullptr;
		pTech = _fx->GetTechniqueByName("NormalTech");

		// 렌더링
		D3DX11_TECHNIQUE_DESC techDesc;
		pTech->GetDesc(&techDesc);
		UINT stride = sizeof(MyVertex);
		UINT offset = 0;

		_immediateContext->IASetVertexBuffers(0, 1, &_heightMapVertexBuffer, &stride, &offset);
		_immediateContext->IASetIndexBuffer(_heightMapIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

		for (UINT p = 0; p < techDesc.Passes; ++p)
		{
			if (_isDrawWireFrame)
			{
				pTech->GetPassByIndex(2)->Apply(0, _immediateContext);
			}
			else
			{
				pTech->GetPassByIndex(0)->Apply(0, _immediateContext);
			}
			_immediateContext->DrawIndexed(_meshData->Indices32.size(), 0, 0);

			// Restore default
			_immediateContext->RSSetState(0);

			// 피킹된 삼각형이 존재하는 경우
			if (_pickedTriangle != -1)
			{
				_immediateContext->OMSetDepthStencilState(_pickedStencilState, 0);
				pTech->GetPassByIndex(p)->Apply(0, _immediateContext);
				_immediateContext->DrawIndexed(3, 3 * _pickedTriangle, 0);

				_immediateContext->OMSetDepthStencilState(0, 0);
			}
		}

		_swapChain->Present(0, 0);
	}

	const float changeDelta = 0.1f;
	void DirectXWindow::geometryHeightChange()
	{
#pragma region util

		// 변한 데이터를 동적으로 매핑해줌.
		auto DataMapping = [this]()
		{
			D3D11_MAPPED_SUBRESOURCE mappedData;
			ZeroMemory(&mappedData, sizeof(D3D11_MAPPED_SUBRESOURCE));
			auto hr = _immediateContext->Map(_heightMapVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData);

			memcpy(mappedData.pData, &_meshData->Vertices[0], sizeof(MyVertex) * _meshData->Vertices.size());

			_immediateContext->Unmap(_heightMapIndexBuffer, 0);
		};

		// 거리에 따른 사인 그래프 높이 값을 반환해줌.
		auto GetSinHeight = [&](float stndHeight, float distance) -> float
		{
			if (_pickingRange == 0.f) return FLT_MAX;
			auto normDist = distance / _pickingRange;

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
		if (_pickedTriangle == -1)
		{
			return;
		}

		auto& indices = _meshData->Indices32;
		auto& vertices = _meshData->Vertices;

		// 선택한 삼각형이 있는 상태라면, 인덱스를 가지고 Vertex를 역추적.
		auto& pickedVertexIdx = indices[_pickedTriangle * 3];
		auto& pickedVertex = vertices[pickedVertexIdx];

		// Vertex의 높이를 조정.
		if (_pickingType == (int)OPT_WINDOW_FUNCTIONS::PICKING_RISE_SELECTED)
		{
			// RISE가 눌린 경우, 높이를 올려줌.
			pickedVertex.pos.y += changeDelta;
			pickedVertex.color = getColorByHeight(pickedVertex.pos.y);
		}
		else if (_pickingType == (int)OPT_WINDOW_FUNCTIONS::PICKING_DOWN_SELECTED)
		{
			// DONW이 눌린 경우, 높이를 내려줌.
			pickedVertex.pos.y -= changeDelta;
			pickedVertex.color = getColorByHeight(pickedVertex.pos.y);
		}

		// 역추적한 Vertex에서 가까이 있는 Vertex에 접근
		auto width = _meshData->GetWidthNum();
		auto height = _meshData->GetHeightNum();

		auto vec = getSelectRange();
		for (auto& vertexIdx : vec)
		{
			auto dist = GetDistance(
				pickedVertex.pos.x,
				pickedVertex.pos.z,
				vertices[vertexIdx].pos.x,
				vertices[vertexIdx].pos.z);

			auto newHeight = GetSinHeight(pickedVertex.pos.y, dist);

			// 근처 Vertex 높이 작업.
			if (_pickingType == (int)OPT_WINDOW_FUNCTIONS::PICKING_RISE_SELECTED)
			{
				if (newHeight > vertices[vertexIdx].pos.y)
				{
					vertices[vertexIdx].pos.y = newHeight;
					vertices[vertexIdx].color = getColorByHeight(newHeight);
				}
			}
			else if (_pickingType == (int)OPT_WINDOW_FUNCTIONS::PICKING_DOWN_SELECTED)
			{
				if (newHeight < vertices[vertexIdx].pos.y)
				{
					vertices[vertexIdx].pos.y = newHeight;
					vertices[vertexIdx].color = getColorByHeight(newHeight);
				}
			}
			else if (_pickingType == (int)OPT_WINDOW_FUNCTIONS::PICKING_STND_SELECTED)
			{
				if (newHeight < vertices[vertexIdx].pos.y)
				{
					vertices[vertexIdx].pos.y = pickedVertex.pos.y;
					vertices[vertexIdx].color = getColorByHeight(newHeight);
				}
			}
		}
	}

	XMFLOAT4 DirectXWindow::getColorByHeight(float height)
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

	std::vector<int> DirectXWindow::getSelectRange()
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
		if (_pickedTriangle == -1) return vertexVector;

		auto pickedVertexIdx = _meshData->Indices32[_pickedTriangle * 3];
		auto pickedVertex = _meshData->Vertices[pickedVertexIdx];

		auto width = _meshData->GetWidthNum();
		auto height = _meshData->GetHeightNum();

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

		for (int i = 0; i < _meshData->Vertices.size(); ++i)
		{
			auto dist = GetDistance(
				pickedVertex.pos.x,
				pickedVertex.pos.z,
				_meshData->Vertices[i].pos.x,
				_meshData->Vertices[i].pos.z);

			if (dist < _pickingRange)
			{
				vertexVector.push_back(i);
			}
		}

		return vertexVector;
	}

	void DirectXWindow::pick(int sx, int sy)
	{
		// 메쉬 데이터가 아직 만들어지지 않은 상태라면, 에러로 판단.
		if (_meshData == nullptr) return;

		// 시야 공간에서의 피킹 반직선 계산.
		XMFLOAT4X4 P = _camera->GetProj4x4f();

		float vx = (+2.0f * sx / _clientWidth - 1.0f) / P(0, 0);
		float vy = (-2.0f * sy / _clientHeight + 1.0f) / P(1, 1);

		// 반직선 정보 정의 (Origin은 시야공간에서 원점이다)
		XMVECTOR rayOrigin = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
		XMVECTOR rayDir = XMVectorSet(vx, vy, 1.0f, 0.0f);

		// 시야 공간에서 월드 공간으로 전환하는 행렬 구하기.
		XMMATRIX V = _camera->GetView();
		XMMATRIX invView = XMMatrixInverse(&XMMatrixDeterminant(V), V);

		// 월드 공간에서 Mesh 공간으로 전환하는 행렬 구하기. 
		// (현재상태에서는 Identity Matrix이므로 별로 의미는 없음)
		XMMATRIX W = _world;
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
		_pickedTriangle = -1;
		// 임시로 가장 먼 거리를 잡은 tmin
		float tmin = 10000.0f;

		auto meshIndices = _meshData->Indices32;
		auto meshVertices = _meshData->Vertices;
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
					_pickedTriangle = i;
				}
			}
		}
	}

	void DirectXWindow::onKeyboardInput(float deltaTime)
	{
		auto CameraMoveSpeed = _camera->GetMoveSpeed();
		if (_inputLayer->IsKeyDown(VK_W))
		{
			_camera->Walk(CameraMoveSpeed * deltaTime);
		}
		if (_inputLayer->IsKeyDown(VK_S))
		{
			_camera->Walk(-CameraMoveSpeed * deltaTime);
		}
		if (_inputLayer->IsKeyDown(VK_A))
		{
			_camera->Strafe(-CameraMoveSpeed * deltaTime);
		}
		if (_inputLayer->IsKeyDown(VK_D))
		{
			_camera->Strafe(CameraMoveSpeed * deltaTime);
		}
		if (_inputLayer->IsKeyDown(VK_TAB))
		{
			if (_isDrawWireFrame) _isDrawWireFrame = false;
			else _isDrawWireFrame = true;
		}

		_camera->UpdateViewMatrix();
	}

	void DirectXWindow::onMouseDown(WPARAM btnState, int x, int y)
	{
		_lastMousePos.x = x;
		_lastMousePos.y = y;

		SetCapture(_hThis);

		if ((btnState & MK_LBUTTON) != 0)
		{
			_isLeftMouseDown = true;
			pick(x, y);
		}
	}

	void DirectXWindow::onMouseUp(WPARAM btnState, int x, int y)
	{
		_pickedTriangle = -1;
		_isLeftMouseDown = false;
		ReleaseCapture();
	}

	void DirectXWindow::onMouseMove(WPARAM btnState, int x, int y)
	{
		if ((btnState & MK_LBUTTON) != 0)
		{
			// Make each pixel correspond to a quarter of a degree.
			float dx = XMConvertToRadians(0.25f * static_cast<float>(x - _lastMousePos.x));
			float dy = XMConvertToRadians(0.25f * static_cast<float>(y - _lastMousePos.y));

			_camera->Pitch(dy);
			_camera->RotateY(dx);
		}

		_lastMousePos.x = x;
		_lastMousePos.y = y;
	}

	void DirectXWindow::dataMapping()
	{
		D3D11_MAPPED_SUBRESOURCE mappedData;
		ZeroMemory(&mappedData, sizeof(D3D11_MAPPED_SUBRESOURCE));
		auto hr = _immediateContext->Map(_heightMapVertexBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData);

		memcpy(mappedData.pData, &_meshData->Vertices[0], sizeof(MyVertex) * _meshData->Vertices.size());

		_immediateContext->Unmap(_heightMapIndexBuffer, 0);
	}
}