#pragma once

#include <windows.h>

namespace settings
{
    struct Settings
    {
        // Display settings
        int displayHorizontalBorderSize = 40;
        int displayVerticalBorderSize = 32;
        float displayPixelRatio = 1.0f;
        int displayShaderIndex = 0;
        int displayGigaScreenMode = 0;
        bool displayDarkTheme = true;
        float displayBackgroundColorR = 0.0f;
        float displayBackgroundColorG = 0.0f;
        float displayBackgroundColorB = 0.0f;
        bool displayFullScreen = false;
        bool displayShowRefreshRatePopup = true;

        // Model settings
        int modelId = 100;

        // Tape settings
        bool tapeAutoStartStop = true;
        bool tapeThrottleLoading = true;
        bool tapeInstantLoading = true;

        // Audio settings
        int audioDeviceId = -1;
        int audioAyVolume = 90;
        int audioBeeperVolume = 90;
        int audioTapeVolume = 70;
        int audioAyDcAdjustBufferLength = 20;
        int audioBeeperDcAdjustBufferLength = 40;
        int audioTapeDcAdjustBufferLength = 40;
        
        // Main window settings
        int windowMainWidth = 972;
        int windowMainHeight = 768;
        int windowMainLeft = (GetSystemMetrics(SM_CXSCREEN) - windowMainWidth) / 2;
        int windowMainTop = (GetSystemMetrics(SM_CYSCREEN) - windowMainHeight) / 2;
        int windowMainStatus = SW_SHOWNORMAL;

        // Devices settings
        bool devicesBetaDisk = false;
        bool devicesAY = false;
    };

    extern Settings current;

    void load();
    void save();
}