#pragma once

#include "dc_adjustment_filter.h"

namespace ay_3_8912
{
    constexpr int MAX_AMPLITUDE = 3000;

    extern bool enabled;
	extern DcAdjustmentFilter filterA;
	extern DcAdjustmentFilter filterB;
	extern DcAdjustmentFilter filterC;

    void reset();
    void cleanUp();
    void setRegister(uint8_t reg);
    void setRegisterValue(uint8_t val);
    uint8_t getRegisterValue();
    void tact();
}
