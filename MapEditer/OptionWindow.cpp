#include <Windows.h>
#include "OptionWindow.h"

namespace DXMapEditer
{
	OptionWindow::~OptionWindow()
	{
	}

	void OptionWindow::CreateOptionWindow(HINSTANCE hInst, HWND hWnd)
	{
		_hInst = hInst;
		_ParentHandle = hWnd;

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
		optionWindowHandle = this;

		_hThis = CreateWindow(
			TEXT("Option Window"), NULL, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN,
			0, 0, 0, 0, hWnd, (HMENU)0, hInst, NULL);

		//CreateWindows();
	}

	void OptionWindow::MoveOptionWindow()
	{
		MoveWindow(_hThis, 800, 0, 300, 600, TRUE);
	}

	void OptionWindow::CreateWindows(HWND hWnd)
	{
		CreateWindow(TEXT("button"), TEXT("Camera"), WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
			5, 0, 275, 110, hWnd, (HMENU)0, _hInst, NULL);

		CreateWindow(TEXT("static"), TEXT("Move Speed"), WS_CHILD | WS_VISIBLE,
			20, 20, 100, 25, hWnd, (HMENU)-1, _hInst, NULL);

		_MoveSpeedEdit = CreateWindow(TEXT("edit"), TEXT("300"), WS_CHILD | WS_VISIBLE | WS_BORDER,
			125, 20, 50, 25, hWnd, (HMENU)1, _hInst, NULL);

		CreateWindow(TEXT("button"), TEXT("Go To Origin"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
			20, 50, 155, 25, hWnd, (HMENU)2, _hInst, NULL);
	}

	LRESULT OptionWindowProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
	{
		switch (iMessage)
		{
		case WM_CREATE :
		{
			optionWindowHandle->CreateWindows(hWnd);
			break;
		}

		}

		return (DefWindowProc(hWnd, iMessage, wParam, lParam));
	}
}