#pragma once

namespace tape
{
    void init();
    void cleanUp();
    // void instantLoad();

    bool Started();
    void StartTape();
    uint8_t TapeBit();
}