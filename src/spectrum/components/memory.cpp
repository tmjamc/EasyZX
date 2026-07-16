#include <cstdint>
#include <fstream>
#include <format>

#include "memory.h"
#include "main.h"

namespace memory
{
    // As in Sinclair wiki documentation, the term "bank" refers to CPU address space ranges,
    // and the term "page" refers to RAM/ROM memory areas

    uint8_t** banks = nullptr;
	uint8_t** ramPages = nullptr;
	uint8_t** romPages = nullptr;
    int banksCount = 0;
    int ramPagesCount = 0;
    int romPagesCount = 0;
    int activeRomPage = 0;
    int activeRamPage = 0;
    int activeScreenPage = 0;
	bool pagingEnabled = false;

    void cleanUp()
    {
        if (ramPages != nullptr && ramPagesCount > 0)
	    {
            for (int i = 0; i < ramPagesCount; ++i)
            {
                delete[0x4000] ramPages[i];
                ramPages[i] = nullptr;
            }
            delete[ramPagesCount] ramPages;
            ramPages = nullptr;
        }

	    if (romPages != nullptr && romPagesCount > 0)
	    {
            for (int i = 0; i < romPagesCount; ++i)
            {
                delete[0x4000] romPages[i];
                romPages[i] = nullptr;
            }
            delete[romPagesCount] romPages;
            romPages = nullptr;
        }

        if (banks != nullptr && banksCount > 0)
        {
            delete[banksCount] banks;
            banks = nullptr;
        }
    }

    void init(main::Model model)
    {
        cleanUp();

        // Create RAM pages
        ramPagesCount = model.ramPagesCount;
        ramPages = new uint8_t*[ramPagesCount];
        for (int i = 0; i < ramPagesCount; ++i)
        {
            ramPages[i] = new uint8_t[0x4000];
            
        	// Simulate dirty RAM memory
            for (uint16_t addr = 0x0000; addr < 0x4000; ++addr)
            {
                ramPages[i][addr] = ((rand() % 5) == 0) ? rand() & 0xff : ((i % 8) < 4) ? 0x00 : 0xff;
            }
        }

        // Create ROM pages
        romPagesCount = model.romPagesCount;
        romPages = new uint8_t*[romPagesCount];
        for (int i = 0; i < romPagesCount; ++i)
        {
            romPages[i] = new uint8_t[0x4000];

            // Load ROM from file
            std::ifstream file(std::format("./roms/{}", model.romFileNames[i]), std::ios::in | std::ios::binary);
            if (file.is_open())
            {
                file.read(reinterpret_cast<char*>(romPages[i]), 0x4000);
                file.close();
            }
        }

        // Create banks
        banksCount = model.banksCount;
        banks = new uint8_t*[banksCount];

        // Set default ROM bank
        activeRomPage = model.defaultBankPages[0];
        banks[0] = romPages[activeRomPage];

        // Set default RAM banks
        activeRamPage = model.defaultBankPages[banksCount - 1];
        for (int i = 1; i < banksCount; ++i)
        {
            banks[i] = ramPages[model.defaultBankPages[i]];
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
        banks[0] = romPages[activeRomPage];

        activeScreenPage = ((data & 0x08) == 0) ? 5 : 7;

        activeRamPage = data & 0x07;
        banks[3] = ramPages[activeRamPage];

        if ((data & 0x20) != 0)
        {
            pagingEnabled = false;
        }
    }

    uint8_t read(uint16_t addr)
    {
        return banks[(addr & 0xc000) >> 14][addr & 0x3fff];
    }

    void write(uint16_t addr, uint8_t data)
    {
        if (addr < 0x4000)
        {
            return;
        }

        banks[(addr & 0xc000) >> 14][addr & 0x3fff] = data;
    }
}