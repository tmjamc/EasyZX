#pragma once

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
}