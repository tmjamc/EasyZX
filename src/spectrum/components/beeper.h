#pragma once

#include <cstdint>

namespace beeper
{
    extern int16_t* buffer;

    void init();
    void cleanUp();
    void tact();
}