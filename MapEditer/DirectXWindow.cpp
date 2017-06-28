#include <Windows.h>
#include "DirectXWindow.h"

namespace DXMapEditer
{

	DirectXWindow::~DirectXWindow()
	{

	}

	void DirectXWindow::CreateDXWindow(HINSTANCE hInst, HWND hWnd)
	{
		WNDCLASS WndClass;
		WndClass.cbClsExtra = 0;
		WndClass.cbWndExtra = 0;
		WndClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
		WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
		WndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
		WndClass.hInstance = hInst;
		WndClass.lpfnWndProc = DirectXWindowProc;
		WndClass.lpszClassName = L"MapEditer Window";
		WndClass.lpszMenuName = NULL;
		WndClass.style = CS_HREDRAW | CS_VREDRAW;

		if (!RegisterClass(&WndClass)) return;

		_hThis = CreateWindow(
			TEXT("MapEditer Window"), NULL, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN,
			0, 0, 0, 0, hWnd, (HMENU)0, hInst, NULL);
	}

	void DirectXWindow::MoveDXWindow()
	{
		MoveWindow(_hThis, 0, 0, 800, 600, TRUE);
	}

	void DirectXWindow::SetGridVariables(int mapWidth, int mapHeight, int gridWidth, int gridHeight)
	{
		_MapWidth = mapWidth;
		_MapHeight = mapHeight;
		_GridWidth = gridWidth;
		_GridHeight = gridHeight;
	}

	LRESULT DirectXWindow::DirectXWindowProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
	{
		return (DefWindowProc(hWnd, iMessage, wParam, lParam));
	}

}