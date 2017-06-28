#include <Windows.h>
#include <string>
#include "resource.h"
#include "MainWindow.h"

namespace DXMapEditer
{
	MainWindow::MainWindow(HINSTANCE hInstance, int nCmdShow)
	{
		m_hInst = hInstance;
		m_CmdShow = nCmdShow;

		InitWindow();
		GetInitSetting();
	}

	BOOL MainWindow::InitWindow()
	{
		WNDCLASS WndClass;
		WndClass.cbClsExtra = 0;
		WndClass.cbWndExtra = 0;
		WndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
		WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
		WndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
		WndClass.hInstance = m_hInst;
		WndClass.lpfnWndProc = WndProc;
		WndClass.lpszClassName = m_AppName;
		WndClass.lpszMenuName = NULL;
		WndClass.style = CS_HREDRAW | CS_VREDRAW;

		if (!RegisterClass(&WndClass)) return false;

		m_hWnd = CreateWindow(
			m_AppName, m_AppName, WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT, CW_USEDEFAULT, m_ClientWidth, m_ClientHeight,
			NULL, (HMENU)NULL, m_hInst, NULL);

		ShowWindow(m_hWnd, m_CmdShow);
		return true;
	}

	static int mapWidth = 0;
	static int mapHeight = 0;
	static int gridWidth = 0;
	static int gridHeight = 0;

	void MainWindow::GetInitSetting()
	{
		if (DialogBox(m_hInst, MAKEINTRESOURCE(INIT_DIALOG), m_hWnd, InitDialogProc) == IDOK)
		{
			SetMapWidth(mapWidth);
			SetMapHeight(mapHeight);
			SetGridWidth(gridWidth);
			SetGridHeight(gridHeight);
			InvalidateRect(m_hWnd, NULL, TRUE);
		}
	}

	LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
	{
		switch (iMessage)
		{
		case WM_DESTROY:
			PostQuitMessage(0);
			return 0;

		case WM_CLOSE:
			PostQuitMessage(0);
			return 0;

		default:
			DefWindowProc(hWnd, iMessage, wParam, lParam);
		}
	}

	INT_PTR CALLBACK InitDialogProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
	{
		switch (iMessage)
		{
		case WM_INITDIALOG :
			SetDlgItemInt(hWnd, IDC_MAP_WIDTH, mapWidth, FALSE);
			SetDlgItemInt(hWnd, IDC_MAP_HEIGHT, mapHeight, FALSE);
			SetDlgItemInt(hWnd, IDC_GRID_WIDTH, gridWidth, FALSE);
			SetDlgItemInt(hWnd, IDC_GRID_HEIGHT, gridHeight, FALSE);
			return TRUE;
			
		case WM_COMMAND :
		{
			OutputDebugString((std::to_wstring(LOWORD(wParam)) + L"\n").c_str());
			switch (LOWORD(wParam))
			{
			case 1:
				mapWidth = GetDlgItemInt(hWnd, IDC_MAP_WIDTH, NULL, FALSE);
				mapHeight = GetDlgItemInt(hWnd, IDC_MAP_HEIGHT, NULL, FALSE);
				gridWidth = GetDlgItemInt(hWnd, IDC_GRID_WIDTH, NULL, FALSE);
				gridHeight = GetDlgItemInt(hWnd, IDC_GRID_HEIGHT, NULL, FALSE);
				EndDialog(hWnd, IDOK);
				return TRUE;

			case 2:
				PostQuitMessage(0);
				EndDialog(hWnd, IDCANCEL);
				return FALSE;
			}
			break;
		}
		}

		return FALSE;
	}
}