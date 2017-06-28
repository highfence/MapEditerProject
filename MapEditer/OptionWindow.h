#pragma once

namespace DXMapEditer
{
	class OptionWindow
	{
	public :

		OptionWindow() = default;
		~OptionWindow();

		void WindowSetting(HINSTANCE hInst, HWND hWnd);

	private :

		// Option Window Message Procedure
		static LRESULT CALLBACK OptionWindowProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);

	private :

		HWND _hThis;
	};

}