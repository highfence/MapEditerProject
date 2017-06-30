#include <Windows.h>
#include "MainWindow.h"

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE prevhInstance, LPSTR lpszCmdParam, int nCmdShow)
{
	auto mainWindow = new DXMapEditer::MainWindow(hInstance, nCmdShow);

	mainWindow->Run();

	delete mainWindow;
	return 0;
}