#pragma once

#include <vector>

#include <DirectXMath.h>
#include <d3d11.h>
#include <d3dx11effect.h>
#include <d3dCompiler.h>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "Effects11d.lib")
#pragma comment(lib, "d3dcompiler.lib")

#include "Definition.h"

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

		void			 CreateDXWindow(
							HINSTANCE hInst,
							HWND hWnd);

		void			 MoveDXWindow();

		void			 SetGridVariables(
							int mapWidth,
							int mapHeight,
							int gridWidth,
							int gridHeight);

		void			 CalcProc(const float deltaTime);
		void			 DrawProc(const float deltaTime);

		LRESULT CALLBACK MessageHandler(
							HWND hWnd,
							UINT iMessage,
							WPARAM wParam,	
							LPARAM lParam);

		// Setter
		void			 SetCameraMove(int cameraMoveSpeed);
		void			 SetPickingType(int pickedButton);
		void			 SetSelectRange(int range);
		void			 GridInitialize(int init);
		void			 CheckoutWireframe(int flag);
		void			 GoCameraToOrigin(int flag);
		
	private :

		// DirectX Fuctions
		bool			 createDeviceAndSwapChain();
		bool			 createRenderTargetView();
		bool			 createViewPort();
		bool			 createDepthStencilTexture();
		bool			 createEffectShader();
		bool			 buildGeometryBuffers();
		bool			 createConstantBuffer();
		bool			 createRenderState(D3D11_FILL_MODE fillMode, D3D11_CULL_MODE cullMode);
		void			 loadTexture();

		void			 checkDrawEnabled();
		void			 cleanupDevice();
		void			 calculateMatrixForHeightMap(const float deltaTime);
		void			 geometryHeightChange();
		XMFLOAT4		 getColorByHeight(float height);
		std::vector<int> getSelectRange();
		void			 pick(int sx, int sy);

		// InputFunctions
		void			 onKeyboardInput(float deltaTime);
		void			 onMouseDown(WPARAM btnState, int x, int y);
		void			 onMouseUp(WPARAM btnState, int x, int y);
		void			 onMouseMove(WPARAM btnState, int x, int y);
		void			 dataMapping();

	private :

		InputLayer*               _inputLayer            = nullptr;
		Camera*		              _camera                = nullptr;
		POINT                     _lastMousePos;
		float                     _pickingRange          = 25.f;
		const float               _changeDelta           = 0.03f;

		// Window Variable
		HWND                      _hWnd;
		HWND                      _hThis;
		int                       _clientWidth           = 800;
		int                       _clientHeight          = 600;
		
		// Grid Variable
		int                       _mapWidth              = 0;
		int                       _mapHeight             = 0;
		int                       _gridWidth             = 0;
		int                       _gridHeight            = 0;
		uint32_t				  _gridIndexCount        = 0;

		// DirectX Variable
		IDXGISwapChain*           _swapChain             = nullptr;
		ID3D11Device*             _d3dDevice             = nullptr;
		ID3D11DeviceContext*      _immediateContext      = nullptr;
		ID3D11RenderTargetView*   _renderTargetView      = nullptr;

		ID3D11Texture2D*		  _depthStencil          = nullptr;
		ID3D11DepthStencilView*   _depthStencilView      = nullptr;
		D3D_FEATURE_LEVEL		  _featureLevel          = D3D_FEATURE_LEVEL_11_0;

		ID3D11DepthStencilState * _pickedStencilState    = nullptr;
		ID3DX11Effect*            _fx                    = nullptr;
		ID3D11Buffer*			  _heightMapVertexBuffer = nullptr;
		ID3D11Buffer*			  _heightMapIndexBuffer  = nullptr;
		MeshData*				  _meshData              = nullptr;
		ID3D11Buffer*             _constantBuffer        = nullptr;

		ID3D11InputLayout*        _vertexLayout          = nullptr;
		ID3D11RasterizerState*	  _solidRS               = nullptr;
		ID3D11ShaderResourceView* _textureRV             = nullptr;
		ID3D11SamplerState*		  _samplerLinear         = nullptr;

		XMMATRIX                  _world;
		XMMATRIX                  _view;
		XMMATRIX                  _projection;
		bool		              _isDrawWireFrame       = false;
		bool		              _isDrawEnabled         = false;
		bool					  _isLeftMouseDown		 = false;

		/* Picking */
		XMFLOAT4                  _lightDirection        = { XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), };
		XMFLOAT4                  _lightColor            = { XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), };
		UINT	                  _pickedTriangle        = -1;
		int						  _pickingType			 = (int)OPT_WINDOW_FUNCTIONS::PICKING_MOVE_SELECTED;

		// Common Variable
	};

	static DirectXWindow * dxWindowHandler = nullptr;

	// Handling DirectX Window Messages
	LRESULT CALLBACK DirectXWindowProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);
}