#include <Windows.h>
#include "DirectXWindow.h"

namespace DXMapEditer
{

	DirectXWindow::~DirectXWindow()
	{

	}

	void DirectXWindow::CreateDXWindow(HINSTANCE hInst, HWND hWnd)
	{
		_this = CreateWindow(
			TEXT("MapEditer Window"), NULL, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN,
			0, 0, 0, 0, hWnd, (HMENU)0, hInst, NULL);
	}

	void DirectXWindow::MoveDXWindow()
	{
		MoveWindow(_this, 0, 0, 800, 600, TRUE);
	}

	void DirectXWindow::SetGridVariables(int mapWidth, int mapHeight, int gridWidth, int gridHeight)
	{
		_MapWidth = mapWidth;
		_MapHeight = mapHeight;
		_GridWidth = gridWidth;
		_GridHeight = gridHeight;
	}

}