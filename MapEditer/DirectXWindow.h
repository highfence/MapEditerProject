#pragma once

#include <vector>

#include <DirectXMath.h>
#include <d3d11.h>
#include <d3dx11effect.h>
#include <d3dCompiler.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "Effects11d.lib")
#pragma comment(lib, "d3dcompiler.lib")


namespace DXMapEditer
{
	class MeshData;
	class Camera;
	class MyTimer;
	class InputLayer;

	using XMFLOAT4 = DirectX::XMFLOAT4;
	using XMMATRIX = DirectX::XMMATRIX;

	class DirectXWindow
	{
	public :
		DirectXWindow() = default;
		~DirectXWindow();

		void CreateDXWindow(HINSTANCE hInst, HWND hWnd);
		void MoveDXWindow();
		void SetGridVariables(int mapWidth, int mapHeight, int gridWidth, int gridHeight);

		void CalcProc(const float deltaTime);
		void DrawProc(const float deltaTime);

		LRESULT CALLBACK MessageHandler(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);
		
	private :

		// DirectX Fuctions
		bool CreateDeviceAndSwapChain();
		bool CreateRenderTargetView();
		bool CreateViewPort();
		bool CreateDepthStencilTexture();
		bool CreateEffectShader();
		bool BuildGeometryBuffers();
		bool CreateConstantBuffer();
		bool CreateRenderState(D3D11_FILL_MODE fillMode, D3D11_CULL_MODE cullMode);
		void LoadTexture();

		void CheckDrawEnabled();
		void CleanupDevice();
		void CalculateMatrixForHeightMap(const float deltaTime);
		void GeometryHeightChange(int inputKey);
		XMFLOAT4 GetColorByHeight(float height);
		std::vector<int> GetSelectRange();
		void Pick(int sx, int sy);

		// InputFunctions
		void OnKeyboardInput(float deltaTime);
		void OnMouseDown(WPARAM btnState, int x, int y);
		void OnMouseUp(WPARAM btnState, int x, int y);
		void OnMouseMove(WPARAM btnState, int x, int y);

	private :

		InputLayer* m_pInputLayer = nullptr;
		Camera*		m_pCamera = nullptr;
		POINT m_LastMousePos;
		float m_SelectRange = 25.f;
		const float m_ChangeDelta = 0.1f;

		// Window Variable
		HWND _hWnd;
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
		ID3D11ShaderResourceView* m_pTextureRV = nullptr;
		ID3D11SamplerState*		  m_pSamplerLinear = nullptr;

		XMMATRIX    m_World;
		XMMATRIX    m_View;
		XMMATRIX    m_Projection;
		bool		m_IsDrawWireFrame = false;
		bool		m_IsDrawEnabled = false;

		/* Picking */
		XMFLOAT4 m_LightDirection = { XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), };
		XMFLOAT4 m_LightColor = { XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), };
		UINT	 m_PickedTriangle = -1;

		// Common Variable
	};

	static DirectXWindow * dxWindowHandler = nullptr;

	// Handling DirectX Window Messages
	LRESULT CALLBACK DirectXWindowProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);
}