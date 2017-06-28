#pragma once

namespace DXMapEditer
{
	class DirectXWindow
	{
	public :
		DirectXWindow() = default;
		~DirectXWindow();

		void CreateDXWindow(HINSTANCE hInst, HWND hWnd);
		void MoveDXWindow();

	private :

	private :

		// Window Variable
		HWND _this;
		
	};

}