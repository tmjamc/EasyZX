#include "ula.h"

namespace ula
{
    int tacksPerFrame;
    int tacksPerLine;
    int tacksToFirstscreenByte;

    void init(main::Model model)
    {
        tacksPerFrame = model.tacksPerFrame;
        tacksPerLine = model.tacksPerLine;
        tacksToFirstscreenByte = model.tacksToFirstscreenByte;
    }
    
    void cleanUp()
    {
    }
}