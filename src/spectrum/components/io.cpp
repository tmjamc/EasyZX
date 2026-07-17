#include "io.h"
#include "ula.h"
#include "memory.h"
#include "beta_disk.h"
#include "wd1793.h"

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

            // BetaDisk
            if (beta_disk::romEnabled)
            {
                switch (port & 0x00ff)
                {
                case 0x001f:
                case 0x003f:
                case 0x005f:
                case 0x007f:
                case 0x00ff:
                    wd1793::ioWrite(port, data);
                    return;
                }
            }

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

            // BetaDisk
            if (beta_disk::romEnabled)
            {
                switch (port & 0x00ff)
                {
                case 0x001f:
                case 0x003f:
                case 0x005f:
                case 0x007f:
                case 0x00ff:
                    return wd1793::ioRead(port);
                }
            }

            // Floating bus
            return ula::readBus();
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