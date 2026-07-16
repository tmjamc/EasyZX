#include "ula.h"
#include "settings.h"
#include "display.h"

namespace ula
{
    int tacksPerFrame;
    int tacksPerLine;
    int tacksToFirstscreenByte;
    int firstBytePixelTackIndex;

    void init(main::Model model)
    {
        tacksPerFrame = model.tacksPerFrame;
        tacksPerLine = model.tacksPerLine;
        tacksToFirstscreenByte = model.tacksToFirstscreenByte;
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
        }

        const int xTack = main::tack + display::GL_MAX_BORDER_SIZE / 2 - tacksToFirstscreenByte % tacksPerLine;
        const int x = (xTack % tacksPerLine) * 2;
        const int y = xTack / tacksPerLine - tacksToFirstscreenByte / tacksPerLine + display::GL_MAX_BORDER_SIZE;

        if (x >= 0 && x < display::GL_DISPLAY_BUFFER_WIDTH && y >= 0 && y < display::DISPLAY_BUFFER_HEIGHT)
        {
            uint32_t* scanLine = display::displayBuffer + y * display::GL_DISPLAY_BUFFER_WIDTH;
            const int pixIndex = (main::tack - firstBytePixelTackIndex) % 4;
    
            if (main::tack >= tacksToFirstscreenByte)
            {
                scanLine[x] = 0xff0000ff;
                scanLine[x + 1] = 0xff0000ff;
            }
        }

    }
}