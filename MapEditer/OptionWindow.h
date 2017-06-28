#pragma once

namespace DXMapEditer
{
	class OptionWindow
	{
	public :
		OptionWindow() = delete;
		OptionWindow(HINSTANCE hInst, int nCmdShow);
		~OptionWindow();

	private :

		bool initWindow();

	private :

		HINSTANCE _hInst;
		HWND _hWnd;
		int _CmdShow = 0;

	};

	LRESULT CALLBACK OptionWindowProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);
}