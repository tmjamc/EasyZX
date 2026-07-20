#pragma once

#include "dc_adjustment_filter.h"

namespace tape
{
    constexpr int MAX_AMPLITUDE = 3000;

    extern bool playing ;
    extern bool pulseSignal;
    extern DcAdjustmentFilter filter;

    void init();
    void cleanUp();
    void load(const char* fileName);
    void play();
    void tact();
}