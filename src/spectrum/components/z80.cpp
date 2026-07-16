#include "z80.h"
#include "memory.h"
#include "ula.h"
#include "io.h"
#include "z80_macros.h"

namespace z80
{
    #define FLAG_C	0x01
    #define FLAG_N	0x02
    #define FLAG_P	0x04
    #define FLAG_V	FLAG_P
    #define FLAG_3	0x08
    #define FLAG_H	0x10
    #define FLAG_5	0x20
    #define FLAG_Z	0x40
    #define FLAG_S	0x80

    Z80Registers registers;

    bool iff2Read = false;
    bool interruptJustEnabled = false;
    bool halted = false;

    const uint8_t _halfcarryAdd[8] = { 0, FLAG_H, FLAG_H, FLAG_H, 0, 0, 0, FLAG_H };
    const uint8_t _halfcarrySub[8] = { 0, 0, FLAG_H, 0, FLAG_H, 0, FLAG_H, FLAG_H };
    const uint8_t _overflowAdd[8] = { 0, 0, 0, FLAG_V, FLAG_V, 0, 0, 0 };
    const uint8_t _overflowSub[8] = { 0, FLAG_V, 0, 0, 0, 0, FLAG_V, 0 };

    uint8_t _sz53[0x100];
    uint8_t _sz53p[0x100];
    uint8_t _parity[0x100];

    static uint8_t peekByte(uint16_t addr)
    {
        CONTEND(addr, 3);
        return memory::read(addr);
    }

    static void pokeByte(uint16_t addr, uint8_t data)
    {
        CONTEND(addr, 3);
        memory::write(addr, data);
    }

    void executeInstruction()
    {
        uint8_t lastQ;

        uint8_t opcode = peekByte(PC);
        CONTEND(0, 1);

        iff2Read = false;
        interruptJustEnabled = false;

    end_opcode:

        ++R;
        ++PC;
        lastQ = Q;
        Q = 0;

        switch (opcode)
        {          
 
#include "z80ops.cpp"

        }
    }

}