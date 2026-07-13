#include <windows.h>
#include <stdio.h>
#include <iostream>
#include <wchar.h>

#include "win_app.h"

namespace win_app
{
	BOOL run(HINSTANCE hInstance)
	{
        hInst = hInstance;

        registerClass();
        if (initInstance(SW_SHOWNORMAL) == FALSE)
        {
            return FALSE;
        }

        MSG msg;
        while (GetMessage(&msg, NULL, 0, 0))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
            {
                break;
            }
        }

        return TRUE;
	}

    ATOM registerClass()
    {
        WNDCLASSEXW wcex{};
        wcex.cbSize = sizeof(WNDCLASSEX);
        wcex.style = CS_HREDRAW | CS_VREDRAW;
        wcex.lpfnWndProc = WndProc;
        wcex.hInstance = hInst;
        wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wcex.lpszClassName = windowClassname;

        return RegisterClassExW(&wcex);
    }

    BOOL initInstance(int nCmdShow)
    {
        hWnd = CreateWindowW(windowClassname, windowTitle, WS_OVERLAPPEDWINDOW, windowLocation.x, windowLocation.y, windowSize.cx, windowSize.cy, nullptr, nullptr, hInst, nullptr);

        if (!hWnd)
        {
            std::cerr << "Failed to create window!" << std::endl;
            return FALSE;
        }

        ShowWindow(hWnd, nCmdShow);
        UpdateWindow(hWnd);

        return TRUE;
    }

    LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
    {
        switch (message)
        {

        case WM_DESTROY:
            PostQuitMessage(0);
            break;

        case WM_GETMINMAXINFO:
        {
            MINMAXINFO* mmi = (MINMAXINFO*)lParam;
            mmi->ptMinTrackSize.x = 400;
            mmi->ptMinTrackSize.y = 260;
            break;
        }

        default:
            return DefWindowProc(hWnd, message, wParam, lParam);

        }

        return 0;
    }
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    // Check for parameters
    if (wcscmp(lpCmdLine, L"--console") == 0)
    {
        if (!AllocConsole())
        {
            return -1;
        }

        FILE* dummyFile;

        // Redirect stdin to console input
        if (freopen_s(&dummyFile, "CONIN$", "r", stdin) != 0)
        {
            return -1;
        }
        setvbuf(stdin, NULL, _IONBF, 0);

        // Redirect stdout to console output
        if (freopen_s(&dummyFile, "CONOUT$", "w", stdout) != 0)
        {
            return -1;
        }
        setvbuf(stdout, NULL, _IONBF, 0);

        // Redirect stderr to console output
        if (freopen_s(&dummyFile, "CONOUT$", "w", stderr) != 0)
        {
            return -1;
        }
        setvbuf(stderr, NULL, _IONBF, 0);

        // Synchronize C++ standard streams with C streams
        std::ios::sync_with_stdio(true);

        // Clear error states for C++ streams (required after re-opening)
        std::cout.clear();
        std::cerr.clear();
        std::wcout.clear();
        std::wcerr.clear();
        std::cin.clear();
        std::wcin.clear();

        std::cout << "EasyZX log console" << std::endl;
    }

    // Run application
    return win_app::run(hInstance);
}