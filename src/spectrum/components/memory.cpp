#include <cstdint>
#include <fstream>
#include <format>

#include "memory.h"
#include "main.h"

namespace memory
{
    // As in Sinclair wiki documentation, the term "bank" refers to CPU address space ranges,
    // and the term "page" refers to RAM/ROM memory areas

    uint8_t** bank = nullptr;
	uint8_t** ramPage = nullptr;
	uint8_t** romPage = nullptr;
    int bankCount = 0;
    int ramPageCount = 0;
    int romPageCount = 0;
    int activeRomPage = 0;
    int activeRamPage = 0;
    int activeScreenPage = 0;
	bool pagingEnabled = false;

    void cleanUp()
    {
        if (ramPage != nullptr && ramPageCount > 0)
	    {
            for (int i = 0; i < ramPageCount; ++i)
            {
                delete[0x4000] ramPage[i];
                ramPage[i] = nullptr;
            }
            delete[ramPageCount] ramPage;
            ramPage = nullptr;
        }

	    if (romPage != nullptr && romPageCount > 0)
	    {
            for (int i = 0; i < romPageCount; ++i)
            {
                delete[0x4000] romPage[i];
                romPage[i] = nullptr;
            }
            delete[romPageCount] romPage;
            romPage = nullptr;
        }

        if (bank != nullptr && bankCount > 0)
        {
            delete[bankCount] bank;
            bank = nullptr;
        }
    }

    void init(main::Model model)
    {
        cleanUp();

        // Create RAM pages
        ramPageCount = model.ramPageCount;
        ramPage = new uint8_t*[ramPageCount];
        for (int i = 0; i < ramPageCount; ++i)
        {
            ramPage[i] = new uint8_t[0x4000];
            
        	// Simulate dirty RAM memory
            for (uint16_t addr = 0x0000; addr < 0x4000; ++addr)
            {
                ramPage[i][addr] = ((rand() % 5) == 0) ? rand() & 0xff : ((i % 8) < 4) ? 0x00 : 0xff;
            }
        }

        // Create ROM pages
        romPageCount = model.romPageCount;
        romPage = new uint8_t*[romPageCount];
        for (int i = 0; i < romPageCount; ++i)
        {
            romPage[i] = new uint8_t[0x4000];

            // Load ROM from file
            std::ifstream file(std::format("./roms/%s", model.romFileName[i]), std::ios::in | std::ios::binary);
            if (file.is_open())
            {
                file.read(reinterpret_cast<char*>(romPage[i]), 0x4000);
                file.close();
            }
        }

        // Create banks
        bankCount = model.bankCount;
        bank = new uint8_t*[bankCount];

        // Set default ROM bank
        activeRomPage = model.defaultBankPage[0];
        bank[0] = romPage[activeRomPage];

        // Set default RAM banks
        activeRamPage = model.defaultBankPage[bankCount - 1];
        for (int i = 1; i < bankCount; ++i)
        {
            bank[i] = ramPage[model.defaultBankPage[i]];
        }

        pagingEnabled = model.pagingEnabled;
        activeScreenPage = model.activeScreenPage;
    }

    void setPaging(uint8_t data)
    {
        if (!pagingEnabled)
        {
            return;
        }

        activeRomPage = (data & 0x10) >> 4;
        bank[0] = romPage[activeRomPage];

        activeScreenPage = ((data & 0x08) == 0) ? 5 : 7;

        activeRamPage = data & 0x07;
        bank[3] = ramPage[activeRamPage];

        if ((data & 0x20) != 0)
        {
            pagingEnabled = false;
        }
    }

    uint8_t read(uint16_t addr)
    {
        return bank[(addr & 0xc000) >> 14][addr & 0x3fff];
    }

    void write(uint16_t addr, uint8_t data)
    {
        if (addr < 0x4000)
        {
            return;
        }

        bank[(addr & 0xc000) >> 14][addr & 0x3fff] = data;
    }
}