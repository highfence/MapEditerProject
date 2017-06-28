#include <Windows.h>
#include "MainWindow.h"

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE prevhInstance, LPSTR lpszCmdParam, int nCmdShow)
{
	auto mainWindow = DXMapEditer::MainWindow(hInstance, nCmdShow);

	return 0;
}