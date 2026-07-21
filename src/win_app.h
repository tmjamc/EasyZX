#pragma once

#define WIN32_LEAN_AND_MEAN

#include <windows.h>

namespace win_app
{
	extern HWND hWnd;
	extern HDC hDC;
	extern HGLRC glCtx;
	extern bool running;

	bool init(HINSTANCE hInstance);
	void run();
	void info(const char* msg);
	void error(const char* msg);
}