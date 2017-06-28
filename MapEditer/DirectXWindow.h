#pragma once

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

	private :

		// Window Variable
		HWND _hThis;
		
		// Grid Variable
		int _MapWidth = 0;
		int _MapHeight = 0;
		int _GridWidth = 0;
		int _GridHeight = 0;
	};

}