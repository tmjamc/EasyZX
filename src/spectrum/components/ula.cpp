#include "ula.h"
#include "settings.h"
#include "display.h"
#include "memory.h"
#include "tape.h"

namespace ula
{
    #define ATTR_GET_FLASH(a) (a & (1<<7)) && ((main::currentFrame % 64) < 32)
    #define ATTR_GET_INK(a) (a & 0x07) | ((a & (1<<6)) >> 3)
    #define ATTR_GET_PAPER(a) ((a >> 3) & 0x07) | ((a & (1<<6)) >> 3)
    #define KEY_PRESSED(vKey, bit) if (main::keyStates[vKey] /*|| _zx->app->display->keyboard->keyStates[vKey]*/) { data &= ~(0x01 << bit); }

    constexpr static uint8_t CONTENDED_MEMORY_PATTERN[8] = { 6, 5, 4, 3, 2, 1, 0, 0 };

    int firstBytePixelTactIndex;

    uint8_t portData = 0xff;

    int16_t* floatingBusAddresses = nullptr;
    int floatingBusAddressesLength = 0;
    uint8_t* contendedMemoryTacts = nullptr;
    int contendedMemoryTactsLength = 0;

    static void buildTables()
    {
        /*
        * IMO this is the document with the cleanest explanation for contention and ula timings:
        * https://github.com/kosarev/zx/blob/master/test/screen_timing/SCREEN_TIMING.md
        *
        * The base tick
        * =============
        * The first two pixels of the screen area are displayed at CPU tick 64 * 224 + 4 = 14340.
        *
        * The 64 is the number of scanlines before the screen area begins (the top border) and 224 is the number of CPU ticks per scanline.
        *
        * All ticks are since ~INT becomes active, with the first tick since active ~INT being 0.
        *
        * Border timing
        * =============
        * The border colour value is latched and the first two pixels of the corresponding 8-pixel border chunk are displayed at every tick that is a multiple of 4.
        *
        * Note that Z80 writes ports at T2 of the output cycle. This means to take effect the output cycle shall be started at least one tick ahead of the moment when the new border colour value is supposed to be latched.
        *
        * Pixel pattern and colour attribute timing
        * =========================================
        * The first two bytes of the screen area at addresses 0x4000 and 0x4001 and the first two bytes of the colour attribute area at addresses 0x5800 and 0x5801 are latched during the first memory contention cycle at ticks 14336-14341.
        *
        * Similarly to output cycles, memory write cycles actually write the value at T2, so such cycles too have to come at least one tick ahead of the latching moment.
        *
        * The four bytes read are then displayed during ticks 14340-14347 as a chunk of 16 pixels.
        *
        * Then at tick 14348 subsequent four bytes are read and another chunk of 16 pixels is displayed on the screen.
        *
        * Display, memory contention and ULA reads (floating bus) cycles
        * ==============================================================
        *
        * Tick     Contention         ULA read   Screen area pixels
        * ======   ================   ========   =================================
        * 14,336   6 (until 13,342)   -          -
        * 14,337   5 (until 13,342)   -          -
        * 14,338   4 (until 13,342)   0x4000     -
        * 14,339   3 (until 13,342)   0x5800     -
        * 14,340   2 (until 13,342)   0x4001     0b11000000 from 0x4000 and 0x5800
        * 14,341   1 (until 13,342)   0x5801     0b00110000 from 0x4000 and 0x5800
        * 14,342   -                  -          0b00001100 from 0x4000 and 0x5800
        * 14,343   -                  -          0b00000011 from 0x4000 and 0x5800
        * 14,344   6 (until 14,350)   -          0b11000000 from 0x4001 and 0x5801
        * 14,345   5 (until 14,350)   -          0b00110000 from 0x4001 and 0x5801
        * 14,346   4 (until 14,350)   0x4002     0b00001100 from 0x4001 and 0x5801
        * 14,347   3 (until 14,350)   0x5802     0b00000011 from 0x4001 and 0x5801
        * 14,348   2 (until 14,350)   0x4003     0b11000000 from 0x4002 and 0x5802
        * 14,349   1 (until 14,350)   0x5803     0b00110000 from 0x4002 and 0x5802
        * 14,350   -                  -          0b00001100 from 0x4002 and 0x5802
        * 14,351   -                  -          0b00000011 from 0x4002 and 0x5802
        *
        */

        cleanUp();

        // Build contention table
        if (main::currentModel->contendedMemory)
        {
            contendedMemoryTactsLength = main::currentModel->tactsPerFrame;
            contendedMemoryTacts = new uint8_t[contendedMemoryTactsLength];
            std::fill(contendedMemoryTacts, contendedMemoryTacts + contendedMemoryTactsLength, static_cast<uint8_t>(0));

            int index = 0;
            int tact = main::currentModel->tactsToFirstScreenByte - 5;
            while (tact < main::currentModel->tactsToFirstScreenByte - 5 + 192 * main::currentModel->tactsPerLine)
            {
                contendedMemoryTacts[tact++] = CONTENDED_MEMORY_PATTERN[index % 8];

                if (++index == 128)
                {
                    index = 0;
                    tact += (main::currentModel->tactsPerLine - 128);
                }
            }
        }

        // Build floating bus table
        if (main::currentModel->floatingBus)
        {
            floatingBusAddressesLength = main::currentModel->tactsPerFrame;
            floatingBusAddresses = new int16_t[floatingBusAddressesLength];
            std::fill(floatingBusAddresses, floatingBusAddresses + floatingBusAddressesLength, static_cast<int16_t>(-1));
            int tact = main::currentModel->tactsToFirstScreenByte - 2;
            while (tact < main::currentModel->tactsToFirstScreenByte - 2 + 192 * main::currentModel->tactsPerLine)
            {
                const int scrY = (tact + display::GL_MAX_BORDER_SIZE / 2 - main::currentModel->tactsToFirstScreenByte) / main::currentModel->tactsPerLine;
                int pixelAddr = ((scrY & 0xc0) << 5) | ((scrY & 0x07) << 8) | ((scrY & 0x38) << 2);
                int attrAddr = 0x1800 + ((scrY & ~0x07) << 2);
                for (int i = 0; i < 16; ++i)
                {
                    floatingBusAddresses[tact++] = pixelAddr++;
                    floatingBusAddresses[tact++] = attrAddr++;
                    floatingBusAddresses[tact++] = pixelAddr++;
                    floatingBusAddresses[tact++] = attrAddr++;
                    tact += 4;
                }
                tact += (main::currentModel->tactsPerLine - 128);
            }
        }
    }

    void init()
    {
        firstBytePixelTactIndex = main::currentModel->tactsToFirstScreenByte % 4;
        buildTables();
    }

    void cleanUp()
    {
        if (contendedMemoryTactsLength > 0)
        {
            delete[contendedMemoryTactsLength] contendedMemoryTacts;
            contendedMemoryTacts = nullptr;
            contendedMemoryTactsLength = 0;
        }

        if (floatingBusAddressesLength > 0)
        {
            delete[floatingBusAddressesLength] floatingBusAddresses;
            floatingBusAddresses = nullptr;
            floatingBusAddressesLength = 0;
        }

    }

    void tact()
    {
        main::tact();

        const int xTact = main::currentTact + display::GL_MAX_BORDER_SIZE / 2 - main::currentModel->tactsToFirstScreenByte % main::currentModel->tactsPerLine;
        const int x = (xTact % main::currentModel->tactsPerLine) * 2;
        const int y = xTact / main::currentModel->tactsPerLine - main::currentModel->tactsToFirstScreenByte / main::currentModel->tactsPerLine + display::GL_MAX_BORDER_SIZE;

        // Check if current coordinates are inside the display buffer
        if (x >= 0 && x < display::GL_DISPLAY_BUFFER_WIDTH && y >= 0 && y < display::DISPLAY_BUFFER_HEIGHT)
        {
            uint32_t* scanLine = display::displayBuffer + y * display::GL_DISPLAY_BUFFER_WIDTH;
            const int pixIndex = (main::currentTact - firstBytePixelTactIndex) % 4;

            // Check if current coordinates are inside the screen
            if (x >= display::GL_MAX_BORDER_SIZE && x < display::GL_DISPLAY_BUFFER_WIDTH - display::GL_MAX_BORDER_SIZE &&
                y >= display::GL_MAX_BORDER_SIZE && y < display::GL_DISPLAY_BUFFER_HEIGHT - display::GL_MAX_BORDER_SIZE)
            {
                // Draw whole screen 8 pixels chunk
                if (pixIndex == 0)
                {
                    const int scrX = x - display::GL_MAX_BORDER_SIZE;
                    const int scrY = y - display::GL_MAX_BORDER_SIZE;
                    const uint8_t bytePixels = memory::ramPages[memory::activeScreenPage][((scrY & 0xc0) << 5) | ((scrY & 0x07) << 8) | ((scrY & 0x38) << 2) | (scrX >> 3)];
                    const uint8_t byteAttr = memory::ramPages[memory::activeScreenPage][0x1800 + ((scrY & ~0x07) << 2) | (scrX >> 3)];
                    const uint32_t ink = PALETTE[ATTR_GET_FLASH(byteAttr) ? ATTR_GET_PAPER(byteAttr) : ATTR_GET_INK(byteAttr)];
                    const uint32_t paper = PALETTE[ATTR_GET_FLASH(byteAttr) ? ATTR_GET_INK(byteAttr) : ATTR_GET_PAPER(byteAttr)];

                    for (int i = 0; i < 8; ++i)
                    {
                        scanLine[x + i] = bytePixels & (0x80 >> i) ? ink : paper;
                    }
                }
            }
            else
            {
                // Draw 8 or 2 border pixels chunk depending on the border resolution
                if (main::currentModel->hiresBorder || pixIndex == 0)
                {
                    const uint32_t border = PALETTE[portData & 0x07];
                    
                    const int pixEnd = main::currentModel->hiresBorder ? 2 : 8;
                    for (int i = 0; i < pixEnd; ++i)
                    {
                        scanLine[x + i] = border;
                    }
                }
            }
        }
    }

    void contendedTacts(uint16_t addr, int tacts, bool force)
    {
        if (main::currentModel->contendedMemory)
        {
            if (force || ((addr & 0xc000) == 0x4000 || (main::currentModel->pagingEnabled && (addr & 0xc000) == 0xc000 && (memory::activeRamPage & 0x01))))
            {
                int cTacts = contendedMemoryTacts[main::currentTact];
                while (cTacts-- != 0)
                {
                    tact();
                }
            }
        }

        while (tacts-- != 0)
        {
            tact();
        }
    }

    // ULA IO Contention:
    // Contention | LowBitt | Result          | Pre | Post
    //------------------------------------------------------------
    // Yes        | 1       | C:1 C:1 C:1 C:1 | C 1 | C 1 C 1 C 1
    // No         | 1       | N:4             | 1   | 3
    // Yes        | 0       | C:1 C:3         | C 1 | C 3
    // No         | 0       | N:1 C:3         | 1   | C 3

    void ioPreTacts(uint16_t port)
    {
        contendedTacts(port, 1);
    }
    
    void ioPostTacts(uint16_t port)
    {
        if (!main::currentModel->contendedMemory)
        {
            tact();
            tact();
            return;
        }

        if (port & 0x0001)
        {
            if ((port & 0xc000) == 0x4000 || (main::currentModel->pagingEnabled && (port & 0xc000) == 0xc000 && (memory::activeRamPage & 0x01)))
            {
                contendedTacts(port, 1, true);
                contendedTacts(port, 1, true);
                contendedTacts(port, 0, true);
            }
            else
            {
                tact();
                tact();
            }
        }
        else
        {
            contendedTacts(port, 2, true);
        }
    }

    uint8_t readPort(uint16_t port)
    {
        // Bits 5 and 7 are always one
        uint8_t data = (1 << 7) | (1 << 5) | 0x1f;

        // MIC
        if (tape::pulseSignal)
        {
            data |= 0x40;
        }

        // keyboard
        uint8_t hAddr = (port & 0xff00) >> 8;

        // Caps shift, Z, X, C, V
        if (!(hAddr & 0x01))
        {
            KEY_PRESSED(VK_SHIFT, 0) KEY_PRESSED(VK_BACK, 0) KEY_PRESSED(VK_LEFT, 0) KEY_PRESSED(VK_RIGHT, 0) KEY_PRESSED(VK_UP, 0) KEY_PRESSED(VK_DOWN, 0) KEY_PRESSED(VK_CAPITAL, 0) KEY_PRESSED(VK_ESCAPE, 0)
            KEY_PRESSED('Z', 1)
            KEY_PRESSED('X', 2)
            KEY_PRESSED('C', 3)
            KEY_PRESSED('V', 4)
        }

        // A, S, D, F, G
        if (!(hAddr & 0x02))
        {
            KEY_PRESSED('A', 0)
            KEY_PRESSED('S', 1)
            KEY_PRESSED('D', 2)
            KEY_PRESSED('F', 3)
            KEY_PRESSED('G', 4)
        }

        // Q, W, E, R, T
        if (!(hAddr & 0x04))
        {
            KEY_PRESSED('Q', 0)
            KEY_PRESSED('W', 1)
            KEY_PRESSED('E', 2)
            KEY_PRESSED('R', 3)
            KEY_PRESSED('T', 4)
        }

        // 1, 2, 3, 4, 5
        if (!(hAddr & 0x08))
        {
            KEY_PRESSED('1', 0)
            KEY_PRESSED('2', 1) KEY_PRESSED(VK_CAPITAL, 1)
            KEY_PRESSED('3', 2)
            KEY_PRESSED('4', 3)
            KEY_PRESSED('5', 4) KEY_PRESSED(VK_LEFT, 4)
        }

        // 0, 9, 8, 7, 6
        if (!(hAddr & 0x10))
        {
            KEY_PRESSED('0', 0) KEY_PRESSED(VK_BACK, 0)
            KEY_PRESSED('9', 1) 
            KEY_PRESSED('8', 2) KEY_PRESSED(VK_RIGHT, 2)
            KEY_PRESSED('7', 3) KEY_PRESSED(VK_UP, 3)
            KEY_PRESSED('6', 4) KEY_PRESSED(VK_DOWN, 4)
        }

        // P, O, I, U, Y
        if (!(hAddr & 0x20))
        {
            KEY_PRESSED('P', 0)
            KEY_PRESSED('O', 1)
            KEY_PRESSED('I', 2)
            KEY_PRESSED('U', 3)
            KEY_PRESSED('Y', 4)
        }

        // Ent, L, K, J, H
        if (!(hAddr & 0x40))
        {
            KEY_PRESSED(VK_RETURN, 0)
            KEY_PRESSED('L', 1)
            KEY_PRESSED('K', 2)
            KEY_PRESSED('J', 3)
            KEY_PRESSED('H', 4)
        }

        // Spc, Sym shft, M, N, B
        if (!(hAddr & 0x80))
        {
            KEY_PRESSED(VK_SPACE, 0) KEY_PRESSED(VK_ESCAPE, 0)
            KEY_PRESSED(VK_CONTROL, 1)
            KEY_PRESSED('M', 2)
            KEY_PRESSED('N', 3)
            KEY_PRESSED('B', 4)
        }

        // Store PC address to check if a tape loader is reading the port
        // _latestPortReadPCAddress = _zx->z80->registers.pc.w;

        return data;
    }

    uint8_t readBus()
    {
        if (!main::currentModel->floatingBus || floatingBusAddresses[main::currentTact] == -1)
        {
            return 0xff;
        }

        return memory::ramPages[memory::activeScreenPage][floatingBusAddresses[main::currentTact]];
    }
}