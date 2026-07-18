#include "beeper.h"
#include "main.h"
#include "ula.h"
#include "audio.h"

namespace beeper
{
    int16_t* buffer;

    void init()
    {
        buffer = new int16_t[882]{};
    }

    void cleanUp()
    {
        delete[882] buffer;
    }

    void tact()
    {
        if (main::currentTact == 0)
        {
            // audio::update();
            std::fill(buffer, buffer + 882, static_cast<int16_t>(0));
        }

        const int bufferIndex = (main::currentTact * 882) / main::currentModel->tactsPerFrame;
        buffer[bufferIndex] = (buffer[bufferIndex] + (((ula::portData & 0x10) == 0) ? -8192 : 8192)) / 2;
        // buffer[bufferIndex] = rand() & 0xfff;
    }
}