#include "imgui.h"
#include "realtime_monitor.h"
#include "main.h"
#include "settings.h"
#include "zx_theme.h"
#include "model_selection.h"
#include "giga_screen.h"
#include "disk_drive.h"

namespace realtime_monitor
{
    bool opened = true;

    void render()
    {
        // if (!opened)
        // {
        //     return;
        // }

        ImGui::SetNextWindowSize(ImVec2(300, 80), ImGuiCond_Once);
        if (ImGui::ZXBegin("Realtime monitor", nullptr, ImGuiWindowFlags_NoCollapse))
        {
            widgets::renderModelSelection();
            widgets::renderGigaScreen();
            widgets::renderDiskDrive();
        }
        ImGui::End();
    }
}