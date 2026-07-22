#pragma once
#include <cstdint>

namespace beta_disk
{
    extern bool enabled;
    extern bool romEnabled;
    extern uint8_t* rom;

    void reset();
    void cleanUp();
}