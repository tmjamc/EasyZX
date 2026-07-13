#pragma once

#include <windows.h>

namespace win_app
{
	static HINSTANCE hInst;

	static const TCHAR* windowClassname = L"EasyZXClass";
	static const TCHAR* windowTitle = L"EasyZX";
	static const POINT windowLocation = { CW_USEDEFAULT, 0 };
	static const SIZE windowSize = { 1366, 768 };

	HWND hWnd;

	BOOL run(HINSTANCE hInstance);
	ATOM registerClass();
	BOOL initInstance(int nCmdShow);
	LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
}