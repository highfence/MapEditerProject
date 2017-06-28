#pragma once
#include <memory>

class MyTimer;
class OptionWindow;

namespace DXMapEditer
{
	class MainWindow
	{
	public :

		MainWindow() = delete;
		MainWindow(HINSTANCE hInstance, int nCmdShow);
		~MainWindow();

		void Run();

		/* Getter, Setter */
		void SetMapWidth(int mapWidth) { _MapWidth = mapWidth; };
		void SetMapHeight(int mapHeight) { _MapHeight = mapHeight; };
		void SetGridWidth(int gridWidth) { _GridWidth = gridWidth; };
		void SetGridHeight(int gridHeight) { _GridHeight = gridHeight; };

	private :

		BOOL initWindow();
		void getInitSetting();

		void calcProc(const float deltaTime);
		void drawProc(const float deltaTime);

	private :

		// Window Variable
		HINSTANCE _hInst;
		int _CmdShow = 0;
		LPCTSTR _AppName = L"DirectX Map Editer v1.0.0";
		MSG _Msg;
		HWND _hWnd;
		int _ClientWidth = 1100;
		int _ClientHeight = 600;

		// MapEditer Variable
		int _MapWidth = 300;
		int _MapHeight = 300;
		int _GridWidth = 150;
		int _GridHeight = 150;

		// Common Variable
		std::unique_ptr<MyTimer> _pTimer;
		std::unique_ptr<OptionWindow> _pOptWindow;

	};

	INT_PTR CALLBACK InitDialogProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);
	LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);
}