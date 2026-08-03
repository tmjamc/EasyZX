#pragma once

#include "dc_adjustment_filter.h"

namespace tape
{
    constexpr int MAX_AMPLITUDE = 3000;

    struct Block
    {
        uint8_t type;
        int start;
        std::string description;
        int length;
        std::string getInfo();
    };

    extern bool playing;
    extern bool endOfTape;
    extern bool pulseSignal;
    extern DcAdjustmentFilter filter;
    extern int volume;
    extern std::string fileName;
    extern std::vector<Block> blocks;
    extern int blockIndex;
    extern int stopFrameCount;

    void reset();
    void cleanUp();
    void load(std::string fileName);
    void play();
    void stop();
    void tact();
    float getBlockProgress();
    void instantLoad();
    std::string getCurrentBlockInfo();
}