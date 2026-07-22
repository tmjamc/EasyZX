#include <cstdint>
#include <fstream>

#include "beta_disk.h"
#include "settings.h"

namespace beta_disk
{
    bool enabled = false;
    bool romEnabled = false;
    uint8_t* rom = nullptr;

    void reset()
    {
        cleanUp();

        if (settings::current.devicesBetaDisk && rom == nullptr)
        {
            rom = new uint8_t[0x4000];

            std::ifstream file("./roms/trdos.rom", std::ios::in | std::ios::binary);
            if (file.is_open())
            {
                file.read(reinterpret_cast<char*>(rom), 0x4000);
                file.close();
            }
        }

        enabled = settings::current.devicesBetaDisk;
        romEnabled = false;
    }

    void cleanUp()
    {
        if (rom != nullptr)
        {
            delete[] rom;
        }
    }
}