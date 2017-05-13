#include "stdafx.h"
#include "WinMain.h"
#include "MapEditer.h"

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE PrevhInstance, LPSTR lpszCmdParam, int nCmdShow)
{
	MapEditer* mapEditer = new MapEditer(hInstance, nCmdShow);

	mapEditer->Run();

	delete mapEditer;
	return 0;
}