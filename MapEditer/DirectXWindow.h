#pragma once
#include <d3d11.h>
#include <d3dx11effect.h>
#include <d3dCompiler.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "Effects11d.lib")
#pragma comment(lib, "d3dcompiler.lib")

class MeshData;

namespace DXMapEditer
{
	class DirectXWindow
	{
	public :
		DirectXWindow() = default;
		~DirectXWindow();

		void CreateDXWindow(HINSTANCE hInst, HWND hWnd);
		void MoveDXWindow();
		void SetGridVariables(int mapWidth, int mapHeight, int gridWidth, int gridHeight);

	private :

		// Handling DirectX Window Messages
		static LRESULT CALLBACK DirectXWindowProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);

		// DirectX Fuctions
		bool CreateDeviceAndSwapChain();
		bool CreateRenderTargetView();
		bool CreateViewPort();
		bool CreateDepthStencilTexture();
		bool CreateEffectShader();
		bool BuildGeometryBuffers();
		bool CreateConstantBuffer();
		bool CreateRenderState(D3D11_FILL_MODE fillMode, D3D11_CULL_MODE cullMode);

		void CleanupDevice();

	private :

		// Window Variable
		HWND _hThis;
		int  _ClientWidth  = 800;
		int  _ClientHeight = 600;
		
		// Grid Variable
		int  _MapWidth     = 0;
		int  _MapHeight    = 0;
		int  _GridWidth    = 0;
		int  _GridHeight   = 0;
		uint32_t _GridIndexCount = 0;

		// DirectX Variable
		IDXGISwapChain*           m_pSwapChain = nullptr;
		ID3D11Device*             m_pD3DDevice = nullptr;
		ID3D11DeviceContext*      m_pImmediateContext = nullptr;
		ID3D11RenderTargetView*   m_pRenderTargetView = nullptr;

		ID3D11Texture2D*		  m_pDepthStencil = nullptr;
		ID3D11DepthStencilView*   m_pDepthStencilView = nullptr;
		D3D_FEATURE_LEVEL		  m_FeatureLevel = D3D_FEATURE_LEVEL_11_0;

		ID3D11DepthStencilState * m_pPickedStencilState = nullptr;
		ID3DX11Effect* m_pFX = nullptr;
		ID3D11Buffer*			  m_pHeightMapVertexBuffer = nullptr;
		ID3D11Buffer*			  m_pHeightMapIndexBuffer = nullptr;
		MeshData*				  m_MeshData = nullptr;
		ID3D11Buffer*             m_pConstantBuffer = nullptr;

		ID3D11InputLayout*        m_pVertexLayout = nullptr;
		ID3D11RasterizerState*	  m_pSolidRS = nullptr;
		// Common Variable
	};

}