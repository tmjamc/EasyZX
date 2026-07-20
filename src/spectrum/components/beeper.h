#pragma once

#include <cstdint>

#include "dc_adjustment_filter.h"

namespace beeper
{
    constexpr int MAX_AMPLITUDE = 5000;
    
    extern DcAdjustmentFilter filter;

    void init();
    void cleanUp();
    void tact();
}