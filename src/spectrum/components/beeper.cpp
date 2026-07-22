#include "beeper.h"
#include "ula.h"
#include "settings.h"

namespace beeper
{
    namespace
    {
        int previousBufferIndex = -1;
        int16_t previousSample = 0;
    }
    
    DcAdjustmentFilter filter;
    int volume = 0;

    void reset()
    {
        filter.setBufferPercentLength(50);
        volume = settings::current.audioBeeperVolume * MAX_AMPLITUDE / 200;
    }

    void tact()
    {
        filter.add((ula::portData & 0x10) ? volume : -volume);
    }
}