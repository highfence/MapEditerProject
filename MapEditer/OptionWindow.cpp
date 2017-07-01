#include <Windows.h>
#include "OptionWindow.h"

#define MOVE_SPEED_EDIT 1
#define GO_TO_ORIGIN_BUTTON 2
#define MOVE_RADIO_BUTTON 3
#define RISE_RADIO_BUTTON 4
#define DOWN_RADIO_BUTTON 5
#define STANDARDIZATION_RADIO_BUTTON 6

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
	}

	void OptionWindow::MoveOptionWindow()
	{
		MoveWindow(_hThis, 800, 0, 300, 600, TRUE);
	}

	void OptionWindow::CreateWindows(HWND hWnd)
	{
#pragma region Create Tab Functions

		auto CreateCameraTab = [this](HWND hWnd)
		{
			// Make Camera Tab
			CreateWindow(TEXT("button"), TEXT("Camera"), WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
				5, 0, 275, 90, hWnd, (HMENU)0, _hInst, NULL);

			CreateWindow(TEXT("static"), TEXT("Move Speed"), WS_CHILD | WS_VISIBLE,
				15, 20, 100, 25, hWnd, (HMENU)-1, _hInst, NULL);

			_MoveSpeedEdit = CreateWindow(TEXT("edit"), TEXT("300"), WS_CHILD | WS_VISIBLE | WS_BORDER,
				120, 20, 50, 25, hWnd, (HMENU)MOVE_SPEED_EDIT, _hInst, NULL);

			CreateWindow(TEXT("button"), TEXT("Go To Origin"), WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
				175, 18, 95, 57, hWnd, (HMENU)GO_TO_ORIGIN_BUTTON, _hInst, NULL);

			_WireFrameRadioButton = CreateWindow(TEXT("button"), TEXT("View Wireframe"),
				WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
				15, 50, 155, 25, hWnd, (HMENU)-1, _hInst, NULL);
		};

		auto CreatePickingTab = [this](HWND hWnd)
		{
			// Make Picking Tab
			CreateWindow(TEXT("button"), TEXT("Picking"), WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
				5, 95, 275, 180, hWnd, (HMENU)0, _hInst, NULL);

			_MoveButton = CreateWindow(TEXT("button"), TEXT("Move"), WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON,
				15, 120, 100, 25, hWnd, (HMENU)MOVE_RADIO_BUTTON, _hInst, NULL);

			_RiseButton = CreateWindow(TEXT("button"), TEXT("Rise"), WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON,
				15, 150, 100, 25, hWnd, (HMENU)RISE_RADIO_BUTTON, _hInst, NULL);
			
			_DownButton = CreateWindow(TEXT("button"), TEXT("Down"), WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON,
				15, 180, 100, 25, hWnd, (HMENU)DOWN_RADIO_BUTTON, _hInst, NULL);

			_StandardizationButton = CreateWindow(TEXT("button"), TEXT("STND"),
				WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON,
				15, 210, 100, 25, hWnd, (HMENU)STANDARDIZATION_RADIO_BUTTON, _hInst, NULL);

			CheckRadioButton(hWnd, MOVE_RADIO_BUTTON, STANDARDIZATION_RADIO_BUTTON, MOVE_RADIO_BUTTON);

		};

#pragma endregion

		CreateCameraTab(hWnd);
		CreatePickingTab(hWnd);

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