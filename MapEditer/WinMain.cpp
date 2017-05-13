#include "stdafx.h"
#include "WinMain.h"
#include "Definition.h"
#include "MapEditer.h"

using namespace DirectXFramework;

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE PrevhInstance, LPSTR lpszCmdParam, int nCmdShow)
{
	MapEditer* mapEditer = new MapEditer(hInstance, nCmdShow);

	mapEditer->Run();

	delete mapEditer;
	return 0;
}