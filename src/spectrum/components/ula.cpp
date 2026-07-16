#include "ula.h"
#include "settings.h"
#include "display.h"
#include "memory.h"

namespace ula
{
    #define ATTR_GET_FLASH(a) (a & (1<<7)) && ((main::frame % 64) < 32)
    #define ATTR_GET_INK(a) (a & 0x07) | ((a & (1<<6)) >> 3)
    #define ATTR_GET_PAPER(a) ((a >> 3) & 0x07) | ((a & (1<<6)) >> 3)
    #define KEY_PRESSED(vKey, bit) if (main::keyStates[vKey] /*|| _zx->app->display->keyboard->keyStates[vKey]*/) { data &= ~(0x01 << bit); }

    int firstBytePixelTackIndex;

    uint8_t portData = 0xff;

    void init()
    {
        firstBytePixelTackIndex = main::currentModel->tacksToFirstscreenByte % 4;
    }

    void cleanUp()
    {
    }

    void tack()
    {
        if (++main::tack == main::currentModel->tacksPerFrame)
        {
            main::tack = 0;
            main::waitForNextFrame();
            ++main::frame;
        }

        const int xTack = main::tack + display::GL_MAX_BORDER_SIZE / 2 - main::currentModel->tacksToFirstscreenByte % main::currentModel->tacksPerLine;
        const int x = (xTack % main::currentModel->tacksPerLine) * 2;
        const int y = xTack / main::currentModel->tacksPerLine - main::currentModel->tacksToFirstscreenByte / main::currentModel->tacksPerLine + display::GL_MAX_BORDER_SIZE;

        // Check if current coordinates are inside the display buffer
        if (x >= 0 && x < display::GL_DISPLAY_BUFFER_WIDTH && y >= 0 && y < display::DISPLAY_BUFFER_HEIGHT)
        {
            uint32_t* scanLine = display::displayBuffer + y * display::GL_DISPLAY_BUFFER_WIDTH;
            const int pixIndex = (main::tack - firstBytePixelTackIndex) % 4;

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

    void contendedTacks(uint16_t addr, int tacks)
    {
        if (main::currentModel->contendedMemory && ((addr & 0xc000) == 0x4000 || (main::currentModel->pagingEnabled && (addr & 0xc000) == 0xc000 && (memory::activeRamPage & 0x01))))
        {
            int tacks = 1; // contendedMemoryTacks[main::tack];
            while (tacks-- != 0)
            {
                tack();
            }
        }

        while (tacks-- != 0)
        {
            tack();
        }
    }

    // ULA IO Contention:
    // Contention | LowBitt | Result          | Pre | Post
    //------------------------------------------------------------
    // Yes        | 1       | C:1 C:1 C:1 C:1 | C 1 | C 1 C 1 C 1
    // No         | 1       | N:4             | 1   | 3
    // Yes        | 0       | C:1 C:3         | C 1 | C 3
    // No         | 0       | N:1 C:3         | 1   | C 3

    void preIOTacks(uint16_t port)
    {
        contendedTacks(port, 1);
        tack();
    }
    
    void postIOTacks(uint16_t port)
    {
        if (!main::currentModel->contendedMemory)
        {
            tack();
            tack();
            return;
        }

        if (port & 0x0001)
        {
            if ((port & 0xc000) == 0x4000 || (main::currentModel->pagingEnabled && (port & 0xc000) == 0xc000 && (memory::activeRamPage & 0x01)))
            {
                contendedTacks(port, 1);
                tack();
                contendedTacks(port, 1);
                tack();
                contendedTacks(port, 1);
            }
            else
            {
                tack();
                tack();
            }
        }
        else
        {
            contendedTacks(port, 1);
            tack();
            tack();
        }
    }

    uint8_t readPort(uint16_t port)
    {
        // Bits 5 and 7 are always one
        uint8_t data = (1 << 7) | (1 << 5) | 0x1f;

        // MIC
        // if (_earInput)
        // {
        //     data |= 0x40;
        // }

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
}