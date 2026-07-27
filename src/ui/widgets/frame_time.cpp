#include "zx_theme.h"
#include "frame_time.h"
#include "main.h"

namespace widgets
{
    void renderFrameTime()
    {
        ImGui::ZXLabel("Frame time:");

        ImGui::SameLine();

        if (main::throttle)
        {
            ImGui::Text("N/A");
        }
        else
        {
            ImGui::Text("%d ms", main::frameTime);
        }
    }
}