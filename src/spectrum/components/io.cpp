#include "io.h"
#include "ula.h"
#include "memory.h"

namespace io
{
    static void writeDevicePort(uint16_t port, uint8_t data)
    {
        // Paging
        if (main::currentModel->pagingEnabled && !(port & 0x8002))
        {
            memory::setPaging(data);
            return;
        }

        if (port & 0x0001)
        {
            // // AY-3-8912
            // if ((port & 0xc007) == 0xc005)
            // {
            //     _zx->ay_3_8912->setRegister(data);
            //     return;
            // }

            // if ((port & 0xc007) == 0x8005)
            // {
            //     _zx->ay_3_8912->setRegisterValue(data);
            //     return;
            // }

            // // BetaDisk
            // if (_zx->memory->betaDiskEnabled)
            // {
            //     switch (port & 0x00ff)
            //     {
            //     case 0x001f:
            //     case 0x003f:
            //     case 0x005f:
            //     case 0x007f:
            //         //FDDStep(false);
            //         rvmWD1793Write(_zx->fdd, ((port >> 5) & 0x3), data);
            //         return;
            //     case 0x00ff:
            //         //FDDStep(true);

            //         // Change active disk unit
            //         if (_zx->fdd->diskS != (data & 0x3)) {
            //             _zx->fdd->diskS = data & 0x3;
            //             if (_zx->fdd->disk[_zx->fdd->diskS] != NULL && _zx->fdd->side && _zx->fdd->disk[_zx->fdd->diskS]->sides == 1) _zx->fdd->side = 0;
            //             _zx->fdd->sclConverted = false;
            //         }

            //         if (!(data & 0x4)) {
            //             rvmWD1793Reset(_zx->fdd);
            //         }

            //         if (data & 0x8) _zx->fdd->control |= kRVMWD177XTest;
            //         else _zx->fdd->control &= ~kRVMWD177XTest;

            //         if (data & 0x10) _zx->fdd->side = 0;
            //         else {
            //             if (_zx->fdd->disk[_zx->fdd->diskS] != NULL)
            //                 _zx->fdd->side = _zx->fdd->disk[_zx->fdd->diskS]->sides == 1 ? 0 : 1;
            //             else
            //                 _zx->fdd->side = 1;
            //         }

            //         if (data & 0x40) _zx->fdd->control |= kRVMWD177XDDEN;
            //         else _zx->fdd->control &= ~kRVMWD177XDDEN;
            //         return;
            //     }
            // }

            // No device found
            return;
        }

        // ULA
        ula::portData = data;
    }

    static uint8_t readDevicePort(uint16_t port)
    {
        if (port & 0x0001)
        {
            // // AY-3-8912
            // if ((port & 0xc007) == 0xc005)
            // {
            //     return _zx->ay_3_8912->getRegisterValue();
            // }

            // // BetaDisk
            // if (_zx->memory->betaDiskEnabled)
            // {
            //     switch (port & 0x00ff)
            //     {
            //     case 0x001f:
            //     case 0x003f:
            //     case 0x005f:
            //     case 0x007f:
            //         //FDDStep(false);
            //         return rvmWD1793Read(_zx->fdd, ((port>> 5) & 0x3));
            //     case 0x00ff:
            //         //FDDStep(true);
            //         uint8_t v = 0;
            //         if (_zx->fdd->control & kRVMWD177XDRQ) v |= 0x40;
            //         if (_zx->fdd->control & (kRVMWD177XINTRQ | kRVMWD177XFINTRQ)) v |= 0x80;
            //         return v;
            //     }
            // }

            // Floating bus
            return 0xff; //_zx->ula->getBusData();
        }

        // ULA
        return ula::readPort(port);
    }

    uint8_t read(uint16_t port)
    {
        ula::preIOTacks(port);
        ula::postIOTacks(port);

        const uint8_t data = readDevicePort(port);

        ula::tack();

        return data;
    }

    void write(uint16_t port, uint8_t data)
    {
        ula::preIOTacks(port);

        writeDevicePort(port, data);

        ula::postIOTacks(port);
        ula::tack();
    }
}