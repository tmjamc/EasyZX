#include "imgui.h"
#include "realtime_monitor.h"
#include "main.h"
#include "settings.h"
#include "zx_theme.h"
#include "model_selection.h"
#include "giga_screen.h"
#include "disk_drive.h"
#include "tape_manager.h"

namespace realtime_monitor
{
    bool opened = true;

    void render()
    {
        // if (!opened)
        // {
        //     return;
        // }

        ImGui::SetNextWindowSize(ImVec2(500, 300), ImGuiCond_Once);
        if (ImGui::ZXBegin("Realtime monitor", ImGuiWindowFlags_NoCollapse))
        {
            widgets::renderModelSelection();
            widgets::renderGigaScreen();
            widgets::renderTape();
            widgets::renderDiskDrive();

            ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 10.0f);
            ImGui::Dummy(ImVec2(0.0f, 0.0f));
        }
        ImGui::End();
    }
}