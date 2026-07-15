#pragma once

#include <windows.h>

namespace settings
{
    struct Settings
    {
        // Display settings
        int displayHorizontalBorderSize = 32;
        int displayVerticalBorderSize = 24;
        float displayPixelRatio = 1.0f;
        int displayShaderIndex = 0;
        int displayGigaScreenMode = 0;
        bool displayDarkTheme = true;
        float displayBackgroundColorR = 0.0f;
        float displayBackgroundColorG = 1.0f;
        float displayBackgroundColorB = 0.0f;
        bool displayFullScreen = false;
        bool displayShowRefreshRatePopup = true;

        // Model settings
        int modelId = 0;

        // Tape settings
        bool tapeAutoStartStop = true;
        bool tapeThrottleLoading = true;
        bool tapeInstantLoading = true;

        // Audio settings
        int audioDeviceId = -1;
        int audioAyVolume = 90;
        int audioBeeperVolume = 90;
        int audioTapeVolume = 40;
        int audioAyDcAdjustBufferLength = 20;
        int audioBeeperDcAdjustBufferLength = 40;
        int audioTapeDcAdjustBufferLength = 80;
        
        // Main window settings
        int windowMainLeft = 1119;
        int windowMainTop = 768;
        int windowMainWidth = (GetSystemMetrics(SM_CXSCREEN) - windowMainWidth) / 2;
        int windowMainHeight = (GetSystemMetrics(SM_CYSCREEN) - windowMainHeight) / 2;
        int windowMainStatus = SW_SHOWNORMAL;

        // Devices settings
        bool devicesBetaDisk = false;
        bool devicesAY = false;
    };

    static constexpr const char* settingsFileName = "./EasyZX.ini";

    extern Settings current;

    void load();
    void save();
}