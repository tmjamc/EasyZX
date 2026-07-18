#pragma once

namespace audio
{
    void init();
    void cleanUp();
    void addSample(int16_t sample);
    bool tact();
}