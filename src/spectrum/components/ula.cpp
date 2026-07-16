#include "ula.h"
#include "settings.h"
#include "display.h"
#include "memory.h"

namespace ula
{
    #define ATTR_GET_FLASH(a) (a & (1<<7)) && ((main::frame % 64) < 32)
    #define ATTR_GET_INK(a) (a & 0x07) | ((a & (1<<6)) >> 3)
    #define ATTR_GET_PAPER(a) ((a >> 3) & 0x07) | ((a & (1<<6)) >> 3)

    int tacksPerFrame;
    int tacksPerLine;
    int tacksToFirstscreenByte;
    bool hiresBorder;
    
    int firstBytePixelTackIndex;

    uint8_t portData = 0xff;

    void init(main::Model model)
    {
        tacksPerFrame = model.tacksPerFrame;
        tacksPerLine = model.tacksPerLine;
        tacksToFirstscreenByte = model.tacksToFirstscreenByte;
        hiresBorder = model.hiresBorder;

        firstBytePixelTackIndex = tacksToFirstscreenByte % 4;
    }

    void cleanUp()
    {
    }

    void tack()
    {
        if (++main::tack == tacksPerFrame)
        {
            main::tack = 0;
            main::waitForNextFrame();
            ++main::frame;
        }

        const int xTack = main::tack + display::GL_MAX_BORDER_SIZE / 2 - tacksToFirstscreenByte % tacksPerLine;
        const int x = (xTack % tacksPerLine) * 2;
        const int y = xTack / tacksPerLine - tacksToFirstscreenByte / tacksPerLine + display::GL_MAX_BORDER_SIZE;

        if (x >= 0 && x < display::GL_DISPLAY_BUFFER_WIDTH && y >= 0 && y < display::DISPLAY_BUFFER_HEIGHT)
        {
            uint32_t* scanLine = display::displayBuffer + y * display::GL_DISPLAY_BUFFER_WIDTH;
            const int pixIndex = (main::tack - firstBytePixelTackIndex) % 4;
    
            if (x >= display::GL_MAX_BORDER_SIZE && x < display::GL_DISPLAY_BUFFER_WIDTH - display::GL_MAX_BORDER_SIZE &&
                y >= display::GL_MAX_BORDER_SIZE && y < display::GL_DISPLAY_BUFFER_HEIGHT - display::GL_MAX_BORDER_SIZE)
            {
                if (pixIndex == 0)
                {
                    const int scrX = x - display::GL_MAX_BORDER_SIZE;
                    const int scrY = y - display::GL_MAX_BORDER_SIZE;
                    const uint8_t bytePixels = memory::ramPages[memory::activeScreenPage][((scrY & 0xc0) << 5) | ((scrY & 0x07) << 8) | ((scrY & 0x38) << 2) | (scrX >> 3)];
                    const uint8_t byteAttr = memory::ramPages[memory::activeScreenPage][0x1800 + ((scrY & ~0x07) << 2) | (scrX >> 3)];
                    const uint32_t ink = PALETTE[ATTR_GET_FLASH(byteAttr) ? ATTR_GET_PAPER(byteAttr) : ATTR_GET_INK(byteAttr)];
                    const uint32_t paper = PALETTE[ATTR_GET_FLASH(byteAttr) ? ATTR_GET_INK(byteAttr) : ATTR_GET_PAPER(byteAttr)];

                    // Draw whole byte 8 pixel chunk
                    for (int i = 0; i < 8; ++i)
                    {
                        scanLine[x + i] = bytePixels & (0x80 >> i) ? ink : paper;
                    }
                }
            }
            else
            {
                if (hiresBorder || pixIndex == 0)
                {
                    const uint32_t border = PALETTE[portData & 0x07];
                    
                    // Draw 8 or 2 pixel chunk depending on the border resolution
                    const int pixEnd = hiresBorder ? 2 : 8;
                    for (int i = 0; i < pixEnd; ++i)
                    {
                        scanLine[x + i] = border;
                    }
                }
            }
        }

    }
}