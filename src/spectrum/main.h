#pragma once

#include <thread>
#include <vector>

namespace main
{
    constexpr static int DEFAULT_BANK_PAGES_48K[4] = { 0, 0, 1, 2 };
    constexpr static const char* DEFAULT_ROM_NAMES_48K[1] = { "48.rom" };

    struct Model
    {
        const int tacksPerFrame;
        const int tacksPerLine;
        const int tacksToFirstscreenByte;
        const int interruptSignalTacks;
        const int banksCount;
        const int ramPagesCount;
        const int romPagesCount;
        const bool pagingEnabled;
        const int* defaultBankPages;
        const int activeScreenPage;
        const char* const* romFileNames;
    };

    constexpr static Model SPECTRUM_48K =
    {
        .tacksPerFrame = 69888,
        .tacksPerLine = 224,
        .tacksToFirstscreenByte = 14335,
        .interruptSignalTacks = 32,
        .banksCount = 4,
        .ramPagesCount = 3,
        .romPagesCount = 1,
        .pagingEnabled = false,
        .defaultBankPages = DEFAULT_BANK_PAGES_48K,
        .activeScreenPage = 0,
        .romFileNames = DEFAULT_ROM_NAMES_48K
    };

    extern bool emulationThreadRunning;

    void start();
    void stop();

    // int displayWidth = 352;
    // int displayHeight = 288;

    // // Pentagon
    // //int tacksPerLine = 224;
    // //int tacksPerFrame = 71680;
    // //int TacksToFirstScreenByte = 17983;

    // // 128K
    // int tacksPerLine = 228;
    // int tacksPerFrame = 70908;
    // int TacksToFirstScreenByte = 14361;

    // // 48K
    // //int tacksPerLine = 224;
    // //int tacksPerFrame = 69888;
    // //int TacksToFirstScreenByte = 14335;

    // for (int tack = TacksToFirstScreenByte; tack < TacksToFirstScreenByte + tacksPerLine; ++tack)
    // {
    //     int xTack = tack + 48 / 2 - TacksToFirstScreenByte % tacksPerLine;
    //     int x = (xTack % tacksPerLine) * 2;
    //     int y = xTack / tacksPerLine - TacksToFirstScreenByte / tacksPerLine + 48;
    //     //int y = (xTack - TacksToFirstScreenByte) / tacksPerLine + 48;

    //     std::cout << x << ',' << y << ' ';
    // }

}