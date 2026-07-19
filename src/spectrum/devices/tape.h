#pragma once

namespace tape
{
    extern bool playing;

    void load(const char* fileName);
    void cleanUp();
    void play();
    void tact();
}