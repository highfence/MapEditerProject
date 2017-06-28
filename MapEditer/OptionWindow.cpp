#include <Windows.h>
#include "OptionWindow.h"

namespace DXMapEditer
{
	OptionWindow::~OptionWindow()
	{
	}

	void OptionWindow::WindowSetting(HINSTANCE hInst, HWND hWnd)
	{
		WNDCLASS WndClass;
		WndClass.cbClsExtra = 0;
		WndClass.cbWndExtra = 0;
		WndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
		WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
		WndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
		WndClass.hInstance = hInst;
		WndClass.lpfnWndProc = OptionWindowProc;
		WndClass.lpszClassName = L"Option Window";
		WndClass.lpszMenuName = NULL;
		WndClass.style = CS_HREDRAW | CS_VREDRAW;

		if (!RegisterClass(&WndClass)) return;

		_hThis = CreateWindow(
			TEXT("Option Window"), NULL, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN,
			0, 0, 0, 0, hWnd, (HMENU)0, hInst, NULL);
	}

	LRESULT OptionWindow::OptionWindowProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
	{
		return (DefWindowProc(hWnd, iMessage, wParam, lParam));
	}
}