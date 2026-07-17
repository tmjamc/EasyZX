#include <cstdint>
#include <fstream>

#include "beta_disk.h"
#include "wd1793.h"

namespace beta_disk
{
    bool enabled = false;
    bool romEnabled = false;
    uint8_t* rom;

    void init()
    {
        rom = new uint8_t[0x4000];

        std::ifstream file("./roms/trdos.rom", std::ios::in | std::ios::binary);
        if (file.is_open())
        {
            file.read(reinterpret_cast<char*>(rom), 0x4000);
            file.close();
        }

        wd1793::reset();

        enabled = true;
    }

    void cleanUp()
    {
        if (enabled)
        {
            delete[0x4000] rom;
        }

        enabled = false;
    }
}