#pragma once

#include <thread>
#include <vector>

namespace main
{
    constexpr static int DEFAULT_BANK_PAGES_48K[4] = { 0, 0, 1, 2 };
    constexpr static const char* DEFAULT_ROM_NAMES_48K[1] = { "48.rom" };

    constexpr static int DEFAULT_BANK_PAGES_128K[4] = { 0, 5, 2, 0 };
    constexpr static const char* DEFAULT_ROM_NAMES_128K[2] = { "128-0.rom", "128-1.rom" };

    constexpr static const char* DEFAULT_ROM_NAMES_PENTAGON_128K[2] = { "128p-0.rom", "128p-1.rom" };

    struct Model
    {
        const int tactsPerFrame;
        const int tactsPerLine;
        const int tactsToFirstScreenByte;
        const int tactsToFirstContendedMemory;
        const int interruptSignalTacts;
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
        .tactsPerFrame = 69888,
        .tactsPerLine = 224,
        .tactsToFirstScreenByte = 14340,
        .interruptSignalTacts = 32,
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
        .tactsPerFrame = 70908,
        .tactsPerLine = 228,
        .tactsToFirstScreenByte = 14366,
        .interruptSignalTacts = 36,
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
        .tactsPerFrame = 71680,
        .tactsPerLine = 224,
        .tactsToFirstScreenByte = 17987,
        .interruptSignalTacts = 32,
        .hiresBorder = true,
        .contendedMemory = false,
        .floatingBus = false,
        .banksCount = 4,
        .ramPagesCount = 8,
        .romPagesCount = 2,
        .pagingEnabled = true,
        .defaultBankPages = DEFAULT_BANK_PAGES_128K,
        .activeScreenPage = 5,
        .romFileNames = DEFAULT_ROM_NAMES_PENTAGON_128K
    };

    extern bool emulationThreadReady;
    extern const Model* currentModel;
    extern uint64_t totalTacts;
    extern int currentTact;
    extern int currentFrame;
  	extern bool* keyStates;

    void start();
    void stop();
    void reset(const Model* model);
    void tact();
}