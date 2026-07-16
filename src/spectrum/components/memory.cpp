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
    int activeRomPage = 0;
    int activeRamPage = 0;
    int activeScreenPage = 0;
	bool pagingEnabled = false;

    void cleanUp()
    {
        if (ramPages != nullptr && main::currentModel->ramPagesCount > 0)
	    {
            for (int i = 0; i < main::currentModel->ramPagesCount; ++i)
            {
                delete[0x4000] ramPages[i];
                ramPages[i] = nullptr;
            }
            delete[main::currentModel->ramPagesCount] ramPages;
            ramPages = nullptr;
        }

	    if (romPages != nullptr && main::currentModel->romPagesCount > 0)
	    {
            for (int i = 0; i < main::currentModel->romPagesCount; ++i)
            {
                delete[0x4000] romPages[i];
                romPages[i] = nullptr;
            }
            delete[main::currentModel->romPagesCount] romPages;
            romPages = nullptr;
        }

        if (banks != nullptr && main::currentModel->banksCount > 0)
        {
            delete[main::currentModel->banksCount] banks;
            banks = nullptr;
        }
    }

    void init()
    {
        // Create RAM pages
        // ramPagesCount = model.ramPagesCount;
        ramPages = new uint8_t*[main::currentModel->ramPagesCount];
        for (int i = 0; i < main::currentModel->ramPagesCount; ++i)
        {
            ramPages[i] = new uint8_t[0x4000];
            
        	// Simulate dirty RAM memory
            for (uint16_t addr = 0x0000; addr < 0x4000; ++addr)
            {
                ramPages[i][addr] = ((rand() % 5) == 0) ? rand() & 0xff : ((i % 8) < 4) ? 0x00 : 0xff;
            }
        }

        // Create ROM pages
        // romPagesCount = model.romPagesCount;
        romPages = new uint8_t*[main::currentModel->romPagesCount];
        for (int i = 0; i < main::currentModel->romPagesCount; ++i)
        {
            romPages[i] = new uint8_t[0x4000];

            // Load ROM from file
            std::ifstream file(std::format("./roms/{}", main::currentModel->romFileNames[i]), std::ios::in | std::ios::binary);
            if (file.is_open())
            {
                file.read(reinterpret_cast<char*>(romPages[i]), 0x4000);
                file.close();
            }
        }

        // Create banks
        // banksCount = model.banksCount;
        banks = new uint8_t*[main::currentModel->banksCount];

        // Set default ROM bank
        activeRomPage = main::currentModel->defaultBankPages[0];
        banks[0] = romPages[activeRomPage];

        // Set default RAM banks
        activeRamPage = main::currentModel->defaultBankPages[main::currentModel->banksCount - 1];
        for (int i = 1; i < main::currentModel->banksCount; ++i)
        {
            banks[i] = ramPages[main::currentModel->defaultBankPages[i]];
        }

        pagingEnabled = main::currentModel->pagingEnabled;
        activeScreenPage = main::currentModel->activeScreenPage;
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