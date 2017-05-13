#pragma once

class InputLayer;

class MapEditer
{
public :
	MapEditer() = delete;
	MapEditer(HINSTANCE, int);
	~MapEditer();

	void Run();
	void CleanUpDevice();
	LRESULT CALLBACK MessageHandler(HWND, UINT, WPARAM, LPARAM);

private :

	bool InitWindow();
	bool CreateDeviceAndSwapChain();
	bool CreateRenderTargetView();
	bool CreateViewPort();
	bool InitDirectX();
	bool CreateShader();
	bool CreateVertexBuffer();
	bool CalcProc();
	bool DrawProc();

	/* Window Variables */
	HINSTANCE m_hInstance;
	HWND m_hWnd;
	int m_CmdShow = 0;
	LPCTSTR m_AppName = L"MapEditer";
	MSG m_Message;

	/* DirectX Variables */
	IDXGISwapChain* m_pSwapChain = nullptr;
	ID3D11Device* m_pD3DDevice = nullptr;
	ID3D11DeviceContext* m_pImmediateContext = nullptr;
	ID3D11RenderTargetView* m_pRenderTargetView = nullptr;

	ID3D11VertexShader* m_pVertexShader = nullptr;
	ID3D11InputLayout* m_pVertexLayout = nullptr;
	ID3D11Buffer* m_pVertexBuffer = nullptr;
	ID3D11PixelShader* m_pPixelShader = nullptr;

	D3D_FEATURE_LEVEL m_FeatureLevel = D3D_FEATURE_LEVEL_11_0;

	/* Common Variables */
	int m_Width = 0;
	int m_Height = 0;
	InputLayer* m_InputLayer = nullptr;
};

static MapEditer* editerPtr = nullptr;

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
