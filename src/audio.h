#pragma once

namespace audio
{
    void reset();
    void cleanUp();
    void addSample(int16_t sample);
    bool tact();
}