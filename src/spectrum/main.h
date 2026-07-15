#pragma once

#include <thread>

namespace main
{
    struct Model
    {
        int tacksPerFrame;
        int tacksPerLine;
        int tacksToFirstscreenByte;
        int interruptSignalTacks;
        int bankCount;
        int ramPageCount;
        int romPageCount;
        bool pagingEnabled;
        int* defaultBankPage;
        int activeScreenPage;
        char** romFileName;
    };

    extern bool emulationThreadRunning;

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