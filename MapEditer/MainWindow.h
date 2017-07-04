#pragma once
#include <memory>

class MyTimer;
class DirectXWindow;
class OptionWindow;

namespace DXMapEditer
{
	class MainWindow
	{
	public :

		MainWindow() = delete;
		MainWindow(HINSTANCE hInstance, int nCmdShow);
		~MainWindow();

		void Run();

		// Message Procedure For interesting Messages
		INT_PTR CALLBACK MainWindowProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);

	private :

		BOOL initWindow();
		void getInitSetting();
		void makeWindows(HWND hWnd);

		void calcProc(const float deltaTime);
		void drawProc(const float deltaTime);

	private :

		// Window Variable
		HINSTANCE _hInst;
		int _cmdShow = 0;
		LPCTSTR _appName = L"DirectX Map Editer v1.1.0";
		MSG _msg;
		HWND _hWnd;
		HWND _optionWindowHandle;

		int _clientWidth = 1100;
		int _clientHeight = 600;

		// Common Variable
		std::unique_ptr<MyTimer> _timer;
		std::unique_ptr<DirectXWindow> _dxWindow;
		std::unique_ptr<OptionWindow> _optWindow;
	};

	// Pointer directing Main Window Instance (For WndProc Func)
	static MainWindow* mainWindowHandler = nullptr;

	// Message Handle Procedure
	INT_PTR CALLBACK InitDialogProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);
	LRESULT CALLBACK WndProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);
}