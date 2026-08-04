#include <iostream>
#include <tchar.h>

#include "glad/gl.h"
#include "glad/wgl.h"
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_opengl3.h"

#include "win_app.h"
#include "display.h"
#include "paths.h"
#include "settings.h"
#include "main.h"
#include "zx_theme.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace win_app
{
    HWND hWnd = nullptr;
    HDC hDC = nullptr;
    HGLRC glCtx = nullptr;
    bool running = false;

    namespace
    {
        constexpr wchar_t WINDOW_CLASSNAME[] = L"EasyZXClass";
        constexpr wchar_t WINDOW_TITLE[] = L"EasyZX";
        constexpr int MIN_WINDOW_WIDTH = 400;
        constexpr int MIN_WINDOW_HEIGHT = 260;

        HINSTANCE hInst = nullptr;
        HGLRC tmpCtx = nullptr;
        bool consoleEnabled = false;
        bool windowClassRegistered = false;
        ImGuiContext* imguiContext = nullptr;

        LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
        {
            if (imguiContext != nullptr && ImGui_ImplWin32_WndProcHandler(hwnd, message, wParam, lParam))
            {
                return TRUE;
            }

            switch (message)
            {
            case WM_KEYDOWN:
            case WM_SYSKEYDOWN:
            {
                const auto vk = static_cast<unsigned int>(wParam & 0xffu);
                main::keyStates[vk] = true;
                return 0;
            }

            case WM_KEYUP:
            case WM_SYSKEYUP:
            {
                const auto vk = static_cast<unsigned int>(wParam & 0xffu);
                main::keyStates[vk] = false;
                return 0;
            }

            case WM_DESTROY:
            {
                WINDOWPLACEMENT wp{};
                wp.length = sizeof(wp);
                if (GetWindowPlacement(hwnd, &wp))
                {
                    settings::current.windowMainStatus = wp.showCmd;
                    if (wp.showCmd == SW_SHOWNORMAL)
                    {
                        settings::current.windowMainLeft = wp.rcNormalPosition.left;
                        settings::current.windowMainTop = wp.rcNormalPosition.top;
                        settings::current.windowMainWidth = wp.rcNormalPosition.right - wp.rcNormalPosition.left;
                        settings::current.windowMainHeight = wp.rcNormalPosition.bottom - wp.rcNormalPosition.top;
                    }
                }
                PostQuitMessage(0);
                return 0;
            }

            case WM_GETMINMAXINFO:
            {
                auto* mmi = reinterpret_cast<MINMAXINFO*>(lParam);
                mmi->ptMinTrackSize.x = MIN_WINDOW_WIDTH;
                mmi->ptMinTrackSize.y = MIN_WINDOW_HEIGHT;
                return 0;
            }

            case WM_SIZE:
                if (wParam != SIZE_MINIMIZED)
                {
                    display::setViewport(LOWORD(lParam), HIWORD(lParam));
                }
                return 0;

            default:
                return DefWindowProc(hwnd, message, wParam, lParam);
            }
        }

        bool registerClass()
        {
            info("Registering main window class");

            WNDCLASSEXW wcex{};
            wcex.cbSize = sizeof(wcex);
            wcex.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
            wcex.lpfnWndProc = WndProc;
            wcex.hInstance = hInst;
            wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
            wcex.lpszClassName = WINDOW_CLASSNAME;

            if (RegisterClassExW(&wcex) == 0)
            {
                const DWORD err = GetLastError();
                if (err != ERROR_CLASS_ALREADY_EXISTS)
                {
                    error("Failed to create window class");
                    return false;
                }
            }

            windowClassRegistered = true;
            return true;
        }

        bool initInstance()
        {
            info("Creating main window instance");

            hWnd = CreateWindowW(
                WINDOW_CLASSNAME,
                WINDOW_TITLE,
                WS_OVERLAPPEDWINDOW,
                settings::current.windowMainLeft,
                settings::current.windowMainTop,
                settings::current.windowMainWidth,
                settings::current.windowMainHeight,
                nullptr,
                nullptr,
                hInst,
                nullptr);

            if (hWnd == nullptr)
            {
                error("Failed to create window instance");
                return false;
            }

            return true;
        }

        void cleanUp() noexcept
        {
            info("Cleaning up application");

            if (imguiContext != nullptr)
            {
                ImGui::SetCurrentContext(imguiContext);
                ImGui::ZXThemeCleanUp();
                ImGui::DestroyContext(imguiContext);
                imguiContext = nullptr;
            }

            if (hDC != nullptr)
            {
                wglMakeCurrent(hDC, nullptr);
            }

            if (glCtx != nullptr)
            {
                wglDeleteContext(glCtx);
                glCtx = nullptr;
            }

            if (tmpCtx != nullptr)
            {
                wglDeleteContext(tmpCtx);
                tmpCtx = nullptr;
            }

            if (hWnd != nullptr && hDC != nullptr)
            {
                ReleaseDC(hWnd, hDC);
                hDC = nullptr;
            }

            if (hWnd != nullptr)
            {
                DestroyWindow(hWnd);
                hWnd = nullptr;
            }

            if (windowClassRegistered && hInst != nullptr)
            {
                UnregisterClassW(WINDOW_CLASSNAME, hInst);
                windowClassRegistered = false;
            }
        }

        bool fail(const char* msg)
        {
            error(msg);
            cleanUp();
            return false;
        }

        bool initOpenGL()
        {
            info("Initializing OpenGL");

            hDC = GetDC(hWnd);
            if (hDC == nullptr)
            {
                return fail("Failed to get window device context");
            }

            PIXELFORMATDESCRIPTOR pfd{};
            pfd.nSize = sizeof(pfd);
            pfd.nVersion = 1;
            pfd.dwFlags = PFD_SUPPORT_OPENGL | PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER;
            pfd.iPixelType = PFD_TYPE_RGBA;
            pfd.cColorBits = 32;
            pfd.cDepthBits = 24;
            pfd.cStencilBits = 8;
            pfd.iLayerType = PFD_MAIN_PLANE;

            const int format = ChoosePixelFormat(hDC, &pfd);
            if (format == 0 || SetPixelFormat(hDC, format, &pfd) == FALSE)
            {
                return fail("Failed to set pixel format");
            }

            tmpCtx = wglCreateContext(hDC);
            if (tmpCtx == nullptr)
            {
                return fail("Failed to create temporary OpenGL context");
            }

            if (wglMakeCurrent(hDC, tmpCtx) == FALSE)
            {
                return fail("Failed to activate temporary OpenGL context");
            }

            if (!gladLoaderLoadWGL(hDC) || wglCreateContextAttribsARB == nullptr)
            {
                return fail("Failed to load required WGL extensions");
            }

            const int attributes[] =
            {
                WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
                WGL_CONTEXT_MINOR_VERSION_ARB, 3,
                WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
                0
            };

            glCtx = wglCreateContextAttribsARB(hDC, nullptr, attributes);
            if (glCtx == nullptr)
            {
                return fail("Failed to create OpenGL 3.3 context");
            }

            wglMakeCurrent(hDC, nullptr);
            wglDeleteContext(tmpCtx);
            tmpCtx = nullptr;

            if (wglMakeCurrent(hDC, glCtx) == FALSE)
            {
                return fail("Failed to activate OpenGL context");
            }

            if (!gladLoaderLoadGL())
            {
                return fail("GLAD loader failed");
            }

            wglMakeCurrent(hDC, nullptr);
            return true;
        }

        void initImGui()
        {
            IMGUI_CHECKVERSION();
            imguiContext = ImGui::CreateContext();
            ImGui::SetCurrentContext(imguiContext);

            ImGuiIO& io = ImGui::GetIO();
            io.ConfigFlags &= ~ImGuiConfigFlags_NavEnableGamepad;
            io.ConfigFlags &= ~ImGuiConfigFlags_NavEnableKeyboard;
            io.IniFilename = nullptr;

            ImGui::ZXTheme();
        }

        bool enableConsole()
        {
            if (consoleEnabled)
            {
                return true;
            }

            if (!AllocConsole())
            {
                return false;
            }

            FILE* dummyFile = nullptr;
            if (freopen_s(&dummyFile, "CONIN$", "r", stdin) != 0 ||
                freopen_s(&dummyFile, "CONOUT$", "w", stdout) != 0 ||
                freopen_s(&dummyFile, "CONOUT$", "w", stderr) != 0)
            {
                FreeConsole();
                return false;
            }

            setvbuf(stdin, nullptr, _IONBF, 0);
            setvbuf(stdout, nullptr, _IONBF, 0);
            setvbuf(stderr, nullptr, _IONBF, 0);

            std::ios::sync_with_stdio(true);
            std::cin.clear();
            std::cout.clear();
            std::cerr.clear();
            std::wcin.clear();
            std::wcout.clear();
            std::wcerr.clear();

            consoleEnabled = true;
            info("EasyZX log console");
            return true;
        }
    }

    bool init(HINSTANCE hInstance)
    {
        info("Initializing application");
        hInst = hInstance;

        if (!registerClass() || !initInstance() || !initOpenGL())
        {
            cleanUp();
            return false;
        }

        initImGui();

        info("Showing main window");
        ShowWindow(hWnd, settings::current.windowMainStatus);
        UpdateWindow(hWnd);

        return true;
    }

    void run()
    {
        running = true;
        main::start();

        MSG msg{};
        while (true)
        {
            const BOOL result = GetMessage(&msg, nullptr, 0, 0);
            if (result > 0)
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
                continue;
            }

            if (result == -1)
            {
                error("Message loop failed");
            }
            break;
        }

        running = false;
        main::stop();
    }

    void info(const char* msg)
    {
        if (consoleEnabled)
        {
            std::cout << msg << std::endl;
        }
    }

    void error(const char* msg)
    {
        if (consoleEnabled)
        {
            std::cerr << msg << std::endl;
        }
    }
}

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
    UNREFERENCED_PARAMETER(nCmdShow);

    int argc = 0;
    LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
    if (argv == nullptr)
    {
        return -1;
    }

    for (int i = 0; i < argc; ++i)
    {
        if (wcscmp(argv[i], L"--console") == 0 && !win_app::enableConsole())
        {
            LocalFree(argv);
            return -1;
        }
    }

    LocalFree(argv);

    paths::init();
    settings::load();

    if (!win_app::init(hInstance))
    {
        return -1;
    }

    win_app::run();
    win_app::cleanUp();
    settings::save();

    return 0;
}
