#pragma once

#include <cstdint>

#include "dc_adjustment_filter.h"

namespace beeper
{
    extern DcAdjustmentFilter filter;

    void init();
    void cleanUp();
    void tact();
}