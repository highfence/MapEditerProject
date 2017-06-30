#pragma once

namespace DXMapEditer
{
	class OptionWindow
	{
	public :

		OptionWindow() = default;
		~OptionWindow();

		void CreateOptionWindow(HINSTANCE hInst, HWND hWnd);
		void MoveOptionWindow();
		void CreateWindows(HWND hWnd);

	private :


	private :

		HINSTANCE _hInst;
		HWND _ParentHandle;
		HWND _hThis;
	};

	static OptionWindow* optionWindowHandle = nullptr;

	// Option Window Message Procedure
	LRESULT CALLBACK OptionWindowProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);
}