#include "stdafx.h"
#include "MapEditer.h"
#include "InputLayer.h"

const int tempWidth = 800;
const int tempHeight = 600;

MapEditer::MapEditer(HINSTANCE hInstance, int nCmdShow)
	: m_hInstance(hInstance), m_CmdShow(nCmdShow), m_Width(tempWidth), m_Height(tempHeight)
{
	InitWindow();
	InitDirectX();
	m_InputLayer = new InputLayer;
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
	return false;
}

bool MapEditer::InitDirectX()
{
	HRESULT hr = S_OK;

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
	DXGI_SWAP_CHAIN_DESC     sd;
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

	hr = D3D11CreateDeviceAndSwapChain(
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

	ID3D11Texture2D* pBackBuffer = NULL;
	hr = m_pSwapChain->GetBuffer(0,  
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

	D3D11_VIEWPORT          vp;
	vp.Width = m_Width; 
	vp.Height = m_Height;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0; 
	vp.TopLeftY = 0;
	m_pImmediateContext->RSSetViewports(1, &vp);

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
