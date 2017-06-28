#include <Windows.h>
#include <string>
#include "resource.h"
#include "MyTimer.h"
#include "MainWindow.h"

namespace DXMapEditer
{
	MainWindow::MainWindow(HINSTANCE hInstance, int nCmdShow)
		: _hInst(hInstance), _CmdShow(nCmdShow)
	{
		_pTimer = std::make_unique<MyTimer>();
		_pTimer->Init();

		mainWindowHandler = this;

		getInitSetting();
		initWindow();
	}

	MainWindow::~MainWindow()
	{
		DestroyWindow(_hWnd);
	}

	void MainWindow::Run()
	{
		const float frameTime = 1 / (float)60;
		static float AccTime = 0;
		_pTimer->ProcessTime();

		while (true)
		{
			AccTime += _pTimer->GetElapsedTime();
			if (PeekMessage(&_Msg, NULL, 0, 0, PM_REMOVE))
			{
				if (_Msg.message == WM_QUIT) break;
				TranslateMessage(&_Msg);
				DispatchMessage(&_Msg);
			}
			else
			{
				if (AccTime > frameTime)
				{
					calcProc(AccTime);
					drawProc(AccTime);
					AccTime = 0.f;
				}
			}
		}
	}

	BOOL MainWindow::initWindow()
	{
		WNDCLASS WndClass;
		WndClass.cbClsExtra = 0;
		WndClass.cbWndExtra = 0;
		WndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
		WndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
		WndClass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
		WndClass.hInstance = _hInst;
		WndClass.lpfnWndProc = WndProc;
		WndClass.lpszClassName = _AppName;
		WndClass.lpszMenuName = NULL;
		WndClass.style = CS_HREDRAW | CS_VREDRAW;

		if (!RegisterClass(&WndClass)) return false;

		_hWnd = CreateWindow(
			_AppName, _AppName, WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT, CW_USEDEFAULT, _ClientWidth, _ClientHeight,
			NULL, (HMENU)NULL, _hInst, NULL);

		ShowWindow(_hWnd, _CmdShow);
		return true;
	}

	static int mapWidth = 0;
	static int mapHeight = 0;
	static int gridWidth = 0;
	static int gridHeight = 0;

	void MainWindow::getInitSetting()
	{
		if (DialogBox(_hInst, MAKEINTRESOURCE(INIT_DIALOG), _hWnd, InitDialogProc) == IDOK)
		{
			SetMapWidth(mapWidth);
			SetMapHeight(mapHeight);
			SetGridWidth(gridWidth);
			SetGridHeight(gridHeight);
			InvalidateRect(_hWnd, NULL, TRUE);
		}
	}

	void MainWindow::makeWindows(HWND hWnd)
	{
#pragma region windows func

		auto RegistOptionWindow = [this]()
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

			if (!RegisterClass(&WndClass)) return;
		};

#pragma endregion

		RegistOptionWindow();

		_OptionWindow = CreateWindow(
			TEXT("Option Window"), NULL, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN,
			0, 0, 0, 0, hWnd, (HMENU)0, _hInst, NULL);
	}

	void MainWindow::calcProc(const float deltaTime)
	{
	}

	void MainWindow::drawProc(const float deltaTime)
	{
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
			return DXMapEditer::mainWindowHandler->MainWindowProc(hWnd, iMessage, wParam, lParam);
		}
	}

	// 프로그램 시작에 등장하여 세팅 값을 얻어오는 프로시져.
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

	INT_PTR MainWindow::MainWindowProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
	{
		switch (iMessage)
		{
		case WM_CREATE :
		{
			makeWindows(hWnd);
			break;
		}

		// If Window was moved,
		case WM_SIZE :
			if (wParam != SIZE_MINIMIZED)
			{
				MoveWindow(_OptionWindow, 800, 0, 300, 600, TRUE);
			}
			break;
		}

		return (DefWindowProc(hWnd, iMessage, wParam, lParam));
	}

	// Option Window Message Procedure
	LRESULT OptionWindowProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
	{
		return (DefWindowProc(hWnd, iMessage, wParam, lParam));
	}
}