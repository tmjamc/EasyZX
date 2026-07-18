#include "beeper.h"
#include "ula.h"
#include "tape.h"

namespace beeper
{
    static constexpr int MAX_DC_BUFFER_LENGTH = 882;

    int previousBufferIndex = -1;
    int16_t previousSample = 0;
    DcAdjustmentFilter filter(MAX_DC_BUFFER_LENGTH);

    void init()
    {
        filter.setBufferPercentLength(50);
    }

    void cleanUp()
    {
    }

    void tact()
    {
        // filter.add((ula::portData & 0x10) == 0 ? 0 : 5000);
        filter.add(tape::Started() && tape::TapeBit() == 0 ? 0 : 5000);
    }
}