#include "stdafx.h"
#include "MapEditer.h"
#include "InputLayer.h"
#include "Definition.h"

const int tempWidth = 800;
const int tempHeight = 600;

MapEditer::MapEditer(HINSTANCE hInstance, int nCmdShow)
	: m_hInstance(hInstance), m_CmdShow(nCmdShow), m_Width(tempWidth), m_Height(tempHeight)
{
	InitWindow();
	InitDirectX();

	m_InputLayer = new InputLayer;
	m_InputLayer->Initialize();

	CreateShader();
	CreateVertexBuffer();

	// WndProc이 사용할 수 있도록 포인터 등록.
	editerPtr = this;
}

MapEditer::~MapEditer()
{
	CleanUpDevice();
}

void MapEditer::Run()
{
	while (true)
	{
		if (PeekMessage(&m_Message, NULL, 0, 0, PM_REMOVE))
		{
			if (m_Message.message == WM_QUIT) break;
			TranslateMessage(&m_Message);
			DispatchMessage(&m_Message);
		}
		else
		{
			if (!CalcProc()) break;
			if (!DrawProc()) break;
		}
	}
}

void MapEditer::CleanUpDevice()
{
	if (m_pRenderTargetView)
		m_pRenderTargetView->Release();

	if (m_pImmediateContext)
		m_pImmediateContext->ClearState();

	if (m_pSwapChain)
		m_pSwapChain->Release();

	if (m_pD3DDevice)
		m_pD3DDevice->Release();
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
	vp.Width = m_Width; 
	vp.Height = m_Height;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0; 
	vp.TopLeftY = 0;
	m_pImmediateContext->RSSetViewports(1, &vp);
	return true;
}

bool MapEditer::InitDirectX()
{
	if (m_pVertexBuffer) m_pVertexBuffer->Release();
	if (m_pVertexLayout) m_pVertexLayout->Release();
	if (m_pVertexShader) m_pVertexShader->Release();
	if (m_pPixelShader) m_pPixelShader->Release();

	if (m_pImmediateContext) m_pImmediateContext->ClearState();

	if (!CreateDeviceAndSwapChain()) return false;
	if (!CreateRenderTargetView()) return false;
	if (!CreateViewPort()) return false;

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

	hr = m_pD3DDevice->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), 0, &m_pVertexShader);

	if (FAILED(hr)) return false;

	D3D11_INPUT_ELEMENT_DESC	 layout[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
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
	D3DX11CompileFromFile(L"MyShader.fx", 0, 0, "PS", "ps_5_0", 0, 0, 0,&pPSBlob, &pErrorBlob, 0);
		m_pD3DDevice->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), 0, &m_pPixelShader);
	pPSBlob->Release();

	return true;
}

bool MapEditer::CreateVertexBuffer()
{
	MyVertex vertices[] =
	{
		{ XMFLOAT3(0.0f, 1.0f, 1.0f) },
		{ XMFLOAT3(0.5f, -0.5f, 1.0f) },
		{ XMFLOAT3(-0.5f, -0.5f, 1.0f) },
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

bool MapEditer::CalcProc()
{
	return true;
}

bool MapEditer::DrawProc()
{
	float clearColor[4] = { 0.0f, 0.125f, 0.3f, 1.0f };

	m_pImmediateContext->ClearRenderTargetView(m_pRenderTargetView, clearColor);
	
	// Set Input Assembler 
	m_pImmediateContext->IASetInputLayout(m_pVertexLayout);
	m_pImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	UINT stride = sizeof(MyVertex);
	UINT offset = 0;
	m_pImmediateContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);

	// Set Shader and Draw
	m_pImmediateContext->VSSetShader(m_pVertexShader, NULL, 0);
	m_pImmediateContext->PSSetShader(m_pPixelShader, NULL, 0);
	m_pImmediateContext->Draw(3, 0);

	// Render (백버퍼를 프론트버퍼로 그린다.)
	m_pSwapChain->Present(0, 0);
	return true;
}

LRESULT MapEditer::MessageHandler(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	return (DefWindowProc(hWnd, iMessage, wParam, lParam));
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	switch (iMessage)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}

	return editerPtr->MessageHandler(hWnd, iMessage, wParam, lParam);
}
