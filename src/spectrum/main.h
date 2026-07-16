#pragma once

#include <thread>
#include <vector>

namespace main
{
    constexpr static int DEFAULT_BANK_PAGES_48K[4] = { 0, 0, 1, 2 };
    constexpr static const char* DEFAULT_ROM_NAMES_48K[1] = { "48.rom" };

    constexpr static int DEFAULT_BANK_PAGES_128K[4] = { 0, 5, 2, 0 };
    constexpr static const char* DEFAULT_ROM_NAMES_128K[2] = { "128-0.rom", "128-1.rom" };

    constexpr static const char* DEFAULT_ROM_NAMES_PENTAGON_128K[3] = { "128p-0.rom", "128p-1.rom", "trdos.rom" };

    struct Model
    {
        const int tacksPerFrame;
        const int tacksPerLine;
        const int tacksToFirstScreenByte;
        const int tacksToFirstContendedMemory;
        const int interruptSignalTacks;
        const bool hiresBorder;
        const bool contendedMemory;
        const bool floatingBus;
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
        .tacksToFirstScreenByte = 14340,
        .interruptSignalTacks = 32,
        .hiresBorder = false,
        .contendedMemory = true,
        .floatingBus = true,
        .banksCount = 4,
        .ramPagesCount = 3,
        .romPagesCount = 1,
        .pagingEnabled = false,
        .defaultBankPages = DEFAULT_BANK_PAGES_48K,
        .activeScreenPage = 0,
        .romFileNames = DEFAULT_ROM_NAMES_48K
    };

    constexpr static Model SPECTRUM_128K =
    {
        .tacksPerFrame = 70908,
        .tacksPerLine = 228,
        .tacksToFirstScreenByte = 14366,
        .interruptSignalTacks = 36,
        .hiresBorder = false,
        .contendedMemory = true,
        .floatingBus = true,
        .banksCount = 4,
        .ramPagesCount = 8,
        .romPagesCount = 2,
        .pagingEnabled = true,
        .defaultBankPages = DEFAULT_BANK_PAGES_128K,
        .activeScreenPage = 5,
        .romFileNames = DEFAULT_ROM_NAMES_128K
    };

    constexpr static Model PENTAGON_128K =
    {
        .tacksPerFrame = 71680,
        .tacksPerLine = 224,
        .tacksToFirstScreenByte = 17983,
        .interruptSignalTacks = 32,
        .hiresBorder = true,
        .contendedMemory = false,
        .floatingBus = false,
        .banksCount = 4,
        .ramPagesCount = 8,
        .romPagesCount = 3,
        .pagingEnabled = true,
        .defaultBankPages = DEFAULT_BANK_PAGES_128K,
        .activeScreenPage = 5,
        .romFileNames = DEFAULT_ROM_NAMES_PENTAGON_128K
    };

    extern bool emulationThreadRunning;
    extern const Model* currentModel;
    extern int tack;
    extern int frame;
  	extern bool* keyStates;

    void start();
    void stop();
    void reset(const Model* model);
    void waitForNextFrame();

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

}