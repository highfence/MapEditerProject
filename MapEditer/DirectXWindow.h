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


	private :

		// Window Variable
		HWND _this;
		
		// Grid Variable
		int _MapWidth = 0;
		int _MapHeight = 0;
		int _GridWidth = 0;
		int _GridHeight = 0;
	};

}