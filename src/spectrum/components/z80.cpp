#include "win_app.h"
#include "z80.h"
#include "memory.h"
#include "ula.h"
#include "io.h"
#include "z80_macros.h"

namespace z80
{
    namespace
    {
        bool iff2Read = false;
        bool interruptJustEnabled = false;
        bool halted = false;

        const uint8_t halfcarryAdd[8] = { 0, FLAG_H, FLAG_H, FLAG_H, 0, 0, 0, FLAG_H };
        const uint8_t halfcarrySub[8] = { 0, 0, FLAG_H, 0, FLAG_H, 0, FLAG_H, FLAG_H };
        const uint8_t overflowAdd[8] = { 0, 0, 0, FLAG_V, FLAG_V, 0, 0, 0 };
        const uint8_t overflowSub[8] = { 0, FLAG_V, 0, 0, 0, 0, FLAG_V, 0 };

        uint8_t* sz53 = nullptr;
        uint8_t* sz53p = nullptr;
        uint8_t* parity = nullptr;

        uint8_t contendRead(uint16_t addr)
        {
            CONTEND(addr, 3);
            return memory::read(addr);
        }

        void contendWrite(uint16_t addr, uint8_t data)
        {
            CONTEND(addr, 3);
            memory::write(addr, data);
        }
    }

    Z80Registers registers;

    void reset()
    {
        if (sz53 == nullptr)
        {
            sz53 = new uint8_t[0x100]{};
            sz53p = new uint8_t[0x100]{};
            parity = new uint8_t[0x100]{};

            for (int i = 0; i < 0x100; ++i)
            {
                sz53[i] = i & (FLAG_3 | FLAG_5 | FLAG_S);

                int j = i;
                int p = 0;
                for (int k = 0; k < 8; k++)
                {
                    p ^= (j & 1);
                    j >>= 1;
                }

                parity[i] = (p > 0 ? 0 : FLAG_P);

                sz53p[i] = sz53[i] | parity[i];
            }

            sz53[0] |= FLAG_Z;
            sz53p[0] |= FLAG_Z;
        }

        AF = AF_= 0xffff;
        I = R = R7 = 0;
        PC = 0;
        SP = 0xffff;
        IFF1 = IFF2 = IM = 0;
        halted = false;
        iff2Read = false;
        interruptJustEnabled = false;
        Q = 0;
        BC = DE = HL = 0;
        BC_= DE_= HL_= 0;
        IX = IY = 0;
        MP = 0;

        win_app::info("Z80 -> reset()");
    }

    void cleanUp()
    {
        delete[] sz53;
        delete[] sz53p;
        delete[] parity;
    }

    void executeInstruction()
    {
        uint8_t lastQ;

        uint8_t opcode = contendRead(PC);
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
 
#include "z80_ops.h"

        }
    }

    bool requestInterrupt()
    {
        if (!IFF1 || interruptJustEnabled)
        {
            return false;
        }

        if ( iff2Read && !IS_CMOS)
        {
            F &= ~FLAG_P;
        }

        if (halted)
        {
            ++PC;
            halted = false;
        }

        IFF1 = IFF2 = false;

        ++R;

        ula::contendedTacts(0, 7);

        contendWrite(--SP, PCH);
        contendWrite(--SP, PCL);

        switch(IM)
        {
        case 0:
            PC = 0x0038;
            break;
        case 1:
            PC = 0x0038;
            break;
        case 2:
        {
            uint16_t vec = (0x100 * I) + 0xff;
            PCL = contendRead(vec++);
            PCH = contendRead(vec);
            break;
        }
        }

        MP = PC;
        Q = 0;

        return true;
    }

    void xorByte(uint8_t byte)
    {
        XOR(byte);
    }
}