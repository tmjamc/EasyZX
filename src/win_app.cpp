#include <iostream>
#include <tchar.h>

#include "glad/gl.h"
#include "glad/wgl.h"
#include "win_app.h"
#include "display.h"
#include "settings.h"

namespace win_app
{
    #define ERROR_RESULT(msg)\
    cleanUp();\
	std::cerr << msg << std::endl;\
	return false

    HINSTANCE hInst;

    const char* windowClassname = "EasyZXClass";
    const char* windowTitle = "EasyZX";
    const POINT windowLocation = { CW_USEDEFAULT, 0 };
    const SIZE windowSize = { 1366, 768 };

    HWND hWnd = nullptr;
    HDC hDC = nullptr;
    HGLRC tmpCtx = nullptr;
    HGLRC glCtx = nullptr;

    bool running = false;

    static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
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

        case WM_SIZE:
            display::setViewport(LOWORD(lParam), HIWORD(lParam));
            [[fallthrough]];

        default:
            return DefWindowProc(hWnd, message, wParam, lParam);

        }

        return 0;
    }

    static bool registerClass()
    {
        WNDCLASSEXA wcex{};
        wcex.cbSize = sizeof(WNDCLASSEXA);
        wcex.style = CS_HREDRAW | CS_VREDRAW;
        wcex.lpfnWndProc = WndProc;
        wcex.hInstance = hInst;
        wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
        wcex.lpszClassName = windowClassname;

        if (RegisterClassExA(&wcex) == FALSE)
        {
            std::cerr << "Failed to create window class" << std::endl;
            return false;
        }

        return true;
    }

    static bool initInstance()
    {
        hWnd = CreateWindowA(windowClassname, windowTitle, WS_OVERLAPPEDWINDOW, windowLocation.x, windowLocation.y, windowSize.cx, windowSize.cy, nullptr, nullptr, hInst, nullptr);

        if (!hWnd)
        {
            std::cerr << "Failed to create window instance" << std::endl;
            return false;
        }

        return true;
    }

    static void cleanUp()
    {
        if (hDC != nullptr)
        {
            wglMakeCurrent(hDC, nullptr);
        }

        if (tmpCtx != nullptr)
        {
            wglDeleteContext(tmpCtx);
        }

        if (glCtx != nullptr)
        {
            wglDeleteContext(glCtx);
        }

        if (hDC != nullptr)
        {
            ReleaseDC(hWnd, hDC);
        }

        if (hWnd != nullptr)
        {
            DestroyWindow(hWnd);
        }
    }

    static bool initOpenGL()
    {
        // Get device context
        hDC = GetDC(hWnd);
        if (hDC == nullptr)
        {
            ERROR_RESULT("Failed to get window's device context");
        }

        // Set the pixel format
        PIXELFORMATDESCRIPTOR pfd{};
        pfd.nSize = sizeof(pfd);
        pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
        pfd.dwFlags = PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER;
        pfd.iPixelType = PFD_TYPE_RGBA;
        pfd.cColorBits = 32;
        pfd.iLayerType = PFD_MAIN_PLANE;

        int format = ChoosePixelFormat(hDC, &pfd);
        if (format == 0 || SetPixelFormat(hDC, format, &pfd) == FALSE)
        {
            ERROR_RESULT("Failed to set pixel format");
        }

        // Create and enable a temporary OpenGL context to load WGL extensions
        tmpCtx = wglCreateContext(hDC);
        if (tmpCtx == nullptr)
        {
            ERROR_RESULT("Failed to create temporary OpenGL context");
        }

        wglMakeCurrent(hDC, tmpCtx);

        // Load WGL Extensions
        gladLoaderLoadWGL(hDC);

        // Set the desired OpenGL version
        int attributes[] =
        {
            WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
            WGL_CONTEXT_MINOR_VERSION_ARB, 3,
            WGL_CONTEXT_FLAGS_ARB,
            WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
            0
        };

        // Create OpenGL context and delete the temporary one
        glCtx = wglCreateContextAttribsARB(hDC, nullptr, attributes);
        if (glCtx == nullptr)
        {
            ERROR_RESULT("Failed to create OpenGL context");
        }

        wglMakeCurrent(hDC, nullptr);
        wglDeleteContext(tmpCtx);

        // Set OpenGL context
        wglMakeCurrent(hDC, glCtx);

        // Glad loader
        if (!gladLoaderLoadGL())
        {
            ERROR_RESULT("Glad loader failed");
        }

        // Unbind the OpenGL context for now, it will be re-bound when the rendering thread starts
        wglMakeCurrent(hDC, nullptr);

        return true;
    }

    bool init(HINSTANCE hInstance)
    {
        hInst = hInstance;

        // Load settings
        settings::load();

        if (!registerClass())
        {
            return false;
        }

        if (!initInstance())
        {
            return false;
        }

        if (!initOpenGL())
        {
            return false;
        }

        ShowWindow(hWnd, SW_SHOWNORMAL);

        if (UpdateWindow(hWnd) == FALSE)
        {
            return false;
        }

        return true;
    }

    void run()
    {
        running = true;

        display::startThread();

        MSG msg;
        while (GetMessage(&msg, nullptr, 0, 0))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
            {
                break;
            }
        }

        running = false;

        display::stopThread();

        cleanUp();
    }
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    // Parse for parameters
    int argc;
    LPWSTR *argv = CommandLineToArgvW(GetCommandLineW(), &argc);

    if (argv == nullptr)
    {
        return -1;
    }

    // Check for parameters
    for (int i = 0; i < argc; ++i)
    {
        if (wcscmp(argv[i], L"--console") == 0)
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
            setvbuf(stdin, nullptr, _IONBF, 0);

            // Redirect stdout to console output
            if (freopen_s(&dummyFile, "CONOUT$", "w", stdout) != 0)
            {
                return -1;
            }
            setvbuf(stdout, nullptr, _IONBF, 0);

            // Redirect stderr to console output
            if (freopen_s(&dummyFile, "CONOUT$", "w", stderr) != 0)
            {
                return -1;
            }
            setvbuf(stderr, nullptr, _IONBF, 0);

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
    }

    // Free the memory allocated by CommandLineToArgvW
    LocalFree(argv);

    // Initialize application
    if (!win_app::init(hInstance))
    {
        return -1;
    }

    // Run application
    win_app::run();

    return 0;
}