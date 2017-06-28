#include <Windows.h>
#include "OptionWindow.h"

namespace DXMapEditer
{
	OptionWindow::OptionWindow(HINSTANCE hInst, int nCmdShow)
		: _hInst(hInst), _CmdShow(nCmdShow)
	{
		initWindow();
	}

	OptionWindow::~OptionWindow()
	{
	}

	bool OptionWindow::initWindow()
	{
		WNDCLASS WndClass;
		WndClass.cbClsExtra = 0;
		WndClass.cbWndExtra = 0;
		WndClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
		WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
		WndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
		WndClass.hInstance = _hInst;
		WndClass.lpfnWndProc = OptionWindowProc;
		WndClass.lpszClassName = L"Option Window";
		WndClass.lpszMenuName = NULL;
		WndClass.style = CS_HREDRAW | CS_VREDRAW;

		if (!RegisterClass(&WndClass)) return false;
	}

	LRESULT OptionWindowProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
	{
		return (DefWindowProc(hWnd, iMessage, wParam, lParam));
	}
}