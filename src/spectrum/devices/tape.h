#pragma once

namespace tape
{
    extern bool playing;
    extern bool pulseSignal;

    void load(const char* fileName);
    void cleanUp();
    void play();
    void tact();
}