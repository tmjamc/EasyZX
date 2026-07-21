#include <cstdint>
#include <fstream>
#include <format>

#include "memory.h"
#include "main.h"
#include "ula.h"
#include "win_app.h"

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
                win_app::info(std::format("MEMORY -> Deallocated 0x4000 bytes from RAM page {}", i).c_str());
                ramPages[i] = nullptr;
            }
            delete[main::currentModel->ramPagesCount] ramPages;
            win_app::info(std::format("MEMORY -> Deallocated {} RAM pages", main::currentModel->ramPagesCount).c_str());
            ramPages = nullptr;
        }

	    if (romPages != nullptr && main::currentModel->romPagesCount > 0)
	    {
            for (int i = 0; i < main::currentModel->romPagesCount; ++i)
            {
                delete[0x4000] romPages[i];
                win_app::info(std::format("MEMORY -> Deallocated 0x4000 bytes from ROM page {}", i).c_str());
                romPages[i] = nullptr;
            }
            delete[main::currentModel->romPagesCount] romPages;
            win_app::info(std::format("MEMORY -> Deallocated {} ROM pages", main::currentModel->romPagesCount).c_str());
            romPages = nullptr;
        }

        if (banks != nullptr && main::currentModel->banksCount > 0)
        {
            delete[main::currentModel->banksCount] banks;
            win_app::info(std::format("MEMORY -> Deallocated {} memory banks", main::currentModel->banksCount).c_str());
            banks = nullptr;
        }
    }

    void init()
    {
        // Create RAM pages
        ramPages = new uint8_t*[main::currentModel->ramPagesCount];
        win_app::info(std::format("MEMORY -> Allocated {} RAM pages", main::currentModel->ramPagesCount).c_str());
        for (int i = 0; i < main::currentModel->ramPagesCount; ++i)
        {
            ramPages[i] = new uint8_t[0x4000];
            win_app::info(std::format("MEMORY -> Allocated 0x4000 bytes for RAM page {}", i).c_str());
            
        	// Simulate dirty RAM memory
            for (uint16_t addr = 0x0000; addr < 0x4000; ++addr)
            {
                ramPages[i][addr] = ((rand() % 5) == 0) ? rand() & 0xff : ((i % 8) < 4) ? 0x00 : 0xff;
            }
        }

        // Create ROM pages
        romPages = new uint8_t*[main::currentModel->romPagesCount];
        win_app::info(std::format("MEMORY -> Allocated {} ROM pages", main::currentModel->romPagesCount).c_str());
        for (int i = 0; i < main::currentModel->romPagesCount; ++i)
        {
            romPages[i] = new uint8_t[0x4000];
            win_app::info(std::format("MEMORY -> Allocated 0x4000 bytes for ROM page {}", i).c_str());

            // Load ROM from file
            std::ifstream file(std::format("./roms/{}", main::currentModel->romFileNames[i]), std::ios::in | std::ios::binary);
            if (file.is_open())
            {
                file.read(reinterpret_cast<char*>(romPages[i]), 0x4000);
                file.close();
                win_app::info(std::format("MEMORY -> Loaded file {} into ROM page {}", main::currentModel->romFileNames[i], i).c_str());
            }
        }

        // Create banks
        banks = new uint8_t*[main::currentModel->banksCount];
        win_app::info(std::format("MEMORY -> Allocated {} memory banks", main::currentModel->banksCount).c_str());

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

        const int previousActiveScreenPage = activeScreenPage;
        activeScreenPage = ((data & 0x08) == 0) ? 5 : 7;
        if (previousActiveScreenPage != activeScreenPage)
        {
            ula::gigaScreen = true;
        }

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