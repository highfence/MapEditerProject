#pragma once

class InputLayer;
class MyTimer;
class Camera;

namespace DirectXFramework
{
	class MapEditer
	{
	public:
		MapEditer() = delete;
		MapEditer(HINSTANCE, int);
		~MapEditer();

		void Run();
		void CleanUpDevice();
		LRESULT CALLBACK MessageHandler(HWND, UINT, WPARAM, LPARAM);

	private:

		bool InitWindow();
		bool CreateDeviceAndSwapChain();
		bool CreateRenderTargetView();
		bool CreateViewPort();
		bool InitDirectX();
		void InitMatrix();
		void MakeInnerObjects();
		
		bool DirectXSetting();
		bool CreateShader();
		bool CreateEffectShader();
		bool CreateVertexBuffer();
		bool CreateIndexBuffer();
		bool CreateDepthStencilTexture();
		bool CreateRenderState(D3D11_FILL_MODE, D3D11_CULL_MODE);

		bool CalcProc(float deltaTime);
		void CalculateMatrixForBox(float deltaTime);
		void CalculateMatrixForBox2(float deltaTime);
		void CalculateMatrixForHeightMap(float deltaTime);
		void CalculateMatrix();

		/* Input Processing Functions */
		void OnKeyboardInput(float deltaTime);
		void OnMouseDown(WPARAM btnState, int x, int y);
		void OnMouseUp(WPARAM btnState, int x, int y);
		void OnMouseMove(WPARAM btnState, int x, int y);

		bool DrawProc(float deltaTime);
		bool DrawProcWithEffect(float deltaTime);

		bool CreateConstantBuffer();
		bool LoadTexture();
		bool BuildGeometryBuffers();
		float GetHeight(float x, float z) const;

		void Pick(int sx, int sy);

		/* Window Variables */
		HINSTANCE m_hInstance;
		HWND      m_hWnd;
		int       m_CmdShow = 0;
		LPCTSTR   m_AppName = L"MapEditer";
		MSG       m_Message;

		/* DirectX Variables */
		IDXGISwapChain*           m_pSwapChain        = nullptr;
		ID3D11Device*             m_pD3DDevice        = nullptr;
		ID3D11DeviceContext*      m_pImmediateContext = nullptr;
		ID3D11RenderTargetView*   m_pRenderTargetView = nullptr;

		ID3D11VertexShader*       m_pVertexShader     = nullptr;
		ID3D11InputLayout*        m_pVertexLayout     = nullptr;
		ID3D11Buffer*             m_pVertexBuffer     = nullptr;
		ID3D11PixelShader*        m_pPixelShader      = nullptr;
		ID3D11Buffer*             m_pIndexBuffer      = nullptr;

		ID3D11Buffer*             m_pConstantBuffer   = nullptr;
		D3D_FEATURE_LEVEL		  m_FeatureLevel      = D3D_FEATURE_LEVEL_11_0;

		ID3D11Texture2D*		  m_pDepthStencil	  = nullptr;
		ID3D11DepthStencilView*   m_pDepthStencilView = nullptr;

		ID3D11RasterizerState*	  m_pSolidRS		  = nullptr;
		ID3D11RasterizerState*	  m_pWireframeRS	  = nullptr;

		ID3D11ShaderResourceView* m_pTextureRV		  = nullptr;
		ID3D11SamplerState*		  m_pSamplerLinear	  = nullptr;

		ID3DX11Effect*			  m_pFX				  = nullptr;
		ID3D11Buffer*			  m_pHeightMapVertexBuffer = nullptr;
		ID3D11Buffer*			  m_pHeightMapIndexBuffer = nullptr;
		MeshData*				  m_MeshData		  = nullptr;

		/* Space Matrix */
		XMMATRIX    m_World;
		XMMATRIX	m_World2;
		XMMATRIX    m_View;
		XMMATRIX    m_Projection;

		/* Common Variables */
		int         m_Width        = 0;
		int         m_Height       = 0;
		InputLayer* m_pInputLayer  = nullptr;
		MyTimer*    m_pTimer       = nullptr;
		Camera*		m_pCamera      = nullptr;
		POINT		m_LastMousePos;
		uint32_t    m_GridIndexCount = 0;

		/* Picking */
		XMFLOAT4 m_LightDirection = { XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), };
		XMFLOAT4 m_LightColor     = { XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), };
		UINT	 m_PickedTriangle = -1;
		ID3D11DepthStencilState * m_pPickedStencilState = nullptr;

	};

	static MapEditer* mapEditerHandler = nullptr;

	LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

}