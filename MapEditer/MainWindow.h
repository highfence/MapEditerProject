#pragma once
#include <memory>

class MyTimer;

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
		void SetMapWidth(int mapWidth) { m_MapWidth = mapWidth; };
		void SetMapHeight(int mapHeight) { m_MapHeight = mapHeight; };
		void SetGridWidth(int gridWidth) { m_GridWidth = gridWidth; };
		void SetGridHeight(int gridHeight) { m_GridHeight = gridHeight; };

	private :

		BOOL InitWindow();
		void GetInitSetting();

		void CalcProc(const float deltaTime);
		void DrawProc(const float deltaTime);

	private :

		// Window Variable
		HINSTANCE m_hInst;
		int m_CmdShow = 0;
		LPCTSTR m_AppName = L"DirectX Map Editer v1.0.0";
		MSG m_Msg;
		HWND m_hWnd;
		int m_ClientWidth = 1100;
		int m_ClientHeight = 600;

		// MapEditer Variable
		int m_MapWidth = 300;
		int m_MapHeight = 300;
		int m_GridWidth = 150;
		int m_GridHeight = 150;

		// Common Variable
		std::unique_ptr<MyTimer> m_pTimer;

	};

	INT_PTR CALLBACK InitDialogProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);
	LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);
}