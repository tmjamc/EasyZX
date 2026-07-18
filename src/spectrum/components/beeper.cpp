#include "beeper.h"
#include "ula.h"

namespace beeper
{
    int previousBufferIndex = -1;
    int16_t previousSample = 0;
    DcAdjustmentFilter filter;

    void init()
    {
        filter.setBufferPercentLength(50);
    }

    void cleanUp()
    {
    }

    void tact()
    {
        filter.add((ula::portData & 0x10) == 0 ? -8192 : 8192);
    }
}