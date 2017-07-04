#pragma once
#include <Windows.h>
#include <functional>
#include <unordered_map>

#include "Definition.h"

namespace DXMapEditer
{
	using DetectFunc = std::function<void(int)>;

	class OptionWindow
	{
	public :

		OptionWindow() = default;
		~OptionWindow();

		void CreateOptionWindow(
			HINSTANCE hInst,
			HWND hWnd);

		void MoveOptionWindow();

		void CreateWindows(HWND hWnd);

		void CommandProc(
			WPARAM wParam,
			LPARAM lParam);

		void RegistVariableChangeFunc(
			OPT_WINDOW_FUNCTIONS funcNum,
			DetectFunc functor);

	private :


	private :

		HINSTANCE _hInst;
		HWND _ParentHandle;
		HWND _hThis;

		// Camera Option Handles
		HWND _MoveSpeedEdit;
		HWND _WireFrameRadioButton;
		bool _WireFramed = false;
		
		// Picking Option Handles
		HWND _MoveButton;
		HWND _RiseButton;
		HWND _DownButton;
		HWND _StandardizationButton;
		HWND _RangeEdit;

		// Detect variable changes
		std::unordered_map<OPT_WINDOW_FUNCTIONS, DetectFunc> _funcMap;

	};

	static OptionWindow* optionWindowHandle = nullptr;

	// Option Window Message Procedure
	LRESULT CALLBACK OptionWindowProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);
}