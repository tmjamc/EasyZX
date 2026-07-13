#pragma once

#define WIN32_LEAN_AND_MEAN

#include <windows.h>


namespace win_app
{
	#define ERROR(msg)\
    cleanUp();\
	std::cerr << msg << std::endl;\
	return false

	static HINSTANCE hInst;

	static const TCHAR* windowClassname = L"EasyZXClass";
	static const TCHAR* windowTitle = L"EasyZX";
	static const POINT windowLocation = { CW_USEDEFAULT, 0 };
	static const SIZE windowSize = { 1366, 768 };

	static HWND hWnd = nullptr;
	static HDC hDC = nullptr;
	HGLRC tmpCtx = nullptr;
	HGLRC glCtx = nullptr;

	bool init(HINSTANCE hInstance);
	void run();
}