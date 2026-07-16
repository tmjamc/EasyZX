#pragma once

#include <cstdint>

namespace z80
{
    union Z80RegisterPair
    {
        struct
        {
            uint8_t l, h;
        } b;
        uint16_t w;
    };

    struct Z80Registers
    {
        Z80RegisterPair af, bc, de, hl;
        Z80RegisterPair af_, bc_, de_, hl_;
        Z80RegisterPair ix, iy, sp, pc;
        Z80RegisterPair memPtr;
        uint8_t i;
        uint8_t r;
        uint8_t r7;
        uint8_t q;
        bool iff1;
        bool iff2;
        uint8_t im;
    };

    extern Z80Registers registers;

    void reset();
    void init();
    void cleanUp();
    void executeInstruction();
    bool requestInterrupt();
    void xorByte(uint8_t byte);
}