#include "ui.h"
#include "frame_time.h"
#include "main.h"

namespace widgets
{
    void renderFrameTime()
    {
        ui::Label("Frame time:");

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