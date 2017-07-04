#include <Windows.h>
#include <string>
#include "resource.h"
#include "MyTimer.h"
#include "DirectXWindow.h"
#include "OptionWindow.h"
#include "MainWindow.h"

namespace DXMapEditer
{
	MainWindow::MainWindow(HINSTANCE hInstance, int nCmdShow)
		: _hInst(hInstance), _cmdShow(nCmdShow)
	{
#pragma region UtilFunctions

		auto registFunc = [this]()
		{
			_optWindow->RegistVariableChangeFunc(
				OPT_WINDOW_FUNCTIONS::CAMERA_MOVE_SPEED_CHANGE,
				[this](int changeValue) { _dxWindow->SetCameraMove(changeValue); });

			_optWindow->RegistVariableChangeFunc(
				OPT_WINDOW_FUNCTIONS::CHECK_WIREFRAME,
				[this](int flag) { _dxWindow->CheckoutWireframe(flag); });

			_optWindow->RegistVariableChangeFunc(
				OPT_WINDOW_FUNCTIONS::GO_TO_ORIGIN_CLICKED,
				[this](int flag) { _dxWindow->GoCameraToOrigin(flag); });

			_optWindow->RegistVariableChangeFunc(
				OPT_WINDOW_FUNCTIONS::PICKING_MOVE_SELECTED,
				[this](int pickedButton) { _dxWindow->SetPickingType(pickedButton); });

			_optWindow->RegistVariableChangeFunc(
				OPT_WINDOW_FUNCTIONS::PICKING_RISE_SELECTED,
				[this](int pickedButton) { _dxWindow->SetPickingType(pickedButton); });

			_optWindow->RegistVariableChangeFunc(
				OPT_WINDOW_FUNCTIONS::PICKING_DOWN_SELECTED,
				[this](int pickedButton) { _dxWindow->SetPickingType(pickedButton); });

			_optWindow->RegistVariableChangeFunc(
				OPT_WINDOW_FUNCTIONS::PICKING_STND_SELECTED,
				[this](int pickedButton) { _dxWindow->SetPickingType(pickedButton); });

			_optWindow->RegistVariableChangeFunc(
				OPT_WINDOW_FUNCTIONS::PICKING_RANGE_CHANGE,
				[this](int rangeChange) { _dxWindow->SetSelectRange(rangeChange); });

			_optWindow->RegistVariableChangeFunc(
				OPT_WINDOW_FUNCTIONS::GRID_INITIALIZE_CLICKED,
				[this](int initFlag) { _dxWindow->GridInitialize(initFlag); });
		};

#pragma endregion

		_timer = std::make_unique<MyTimer>();
		_timer->Init();
		_dxWindow = std::make_unique<DirectXWindow>();
		_optWindow = std::make_unique<OptionWindow>();

		mainWindowHandler = this;

		getInitSetting();
		initWindow();
		registFunc();
	}

	MainWindow::~MainWindow()
	{
		DestroyWindow(_hWnd);
	}

	void MainWindow::Run()
	{
		const float frameTime = 1 / (float)60;
		static float AccTime = 0;
		_timer->ProcessTime();

		while (true)
		{
			AccTime += _timer->GetElapsedTime();
			if (PeekMessage(&_msg, NULL, 0, 0, PM_REMOVE))
			{
				if (_msg.message == WM_QUIT) break;
				TranslateMessage(&_msg);
				DispatchMessage(&_msg);
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
		WndClass.lpszClassName = _appName;
		WndClass.lpszMenuName = NULL;
		WndClass.style = CS_HREDRAW | CS_VREDRAW;

		if (!RegisterClass(&WndClass)) return false;

		_hWnd = CreateWindow(
			_appName, _appName, WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT, CW_USEDEFAULT, _clientWidth, _clientHeight,
			NULL, (HMENU)NULL, _hInst, NULL);

		ShowWindow(_hWnd, _cmdShow);
		return true;
	}

	static int mapWidth = 0;
	static int mapHeight = 0;
	static int gridWidth = 0;
	static int gridHeight = 0;

	void MainWindow::getInitSetting()
	{
		// Popup Dialog Box for get input
		if (DialogBox(_hInst, MAKEINTRESOURCE(INIT_DIALOG), _hWnd, InitDialogProc) == IDOK)
		{
			// Deliver input value
			_dxWindow->SetGridVariables(mapWidth, mapHeight, gridWidth, gridHeight);
			InvalidateRect(_hWnd, NULL, TRUE);
		}
	}

	void MainWindow::makeWindows(HWND hWnd)
	{
		_dxWindow->CreateDXWindow(_hInst, hWnd);
		_optWindow->CreateOptionWindow(_hInst, hWnd);
	}

	void MainWindow::calcProc(const float deltaTime)
	{
		_dxWindow.get()->CalcProc(deltaTime);
	}

	void MainWindow::drawProc(const float deltaTime)
	{
		_dxWindow.get()->DrawProc(deltaTime);
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
				_dxWindow->MoveDXWindow();
				_optWindow->MoveOptionWindow();
			}
			break;
		}

		return (DefWindowProc(hWnd, iMessage, wParam, lParam));
	}


}