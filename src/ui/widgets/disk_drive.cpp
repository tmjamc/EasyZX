#include "imgui.h"
#include "disk_drive.h"
#include "wd_1793.h"

namespace widgets
{
    void renderDiskDrive()
    {
        if (!wd_1793::enabled)
        {
            // Begin disabled
        }

        ImGui::AlignTextToFramePadding();

        for (int index = 0; index < 4; ++index)
        {
            const bool empty = wd_1793::disks[index] == nullptr;

            ImGui::Text("Unit %d:", index);

            ImGui::SameLine();
            ImGui::Text(wd_1793::led[index] == 0 ? "IDLE" : (wd_1793::led[index] == 1 ? "READ" : "WRITE"));

            ImGui::SameLine();
            ImGui::TextUnformatted(empty ? "Empty" : wd_1793::disks[index]->fileName.c_str());

            ImGui::SameLine();
            if (ImGui::Button("Insert"))
            {
                // Show disk browse dialog
            }

            ImGui::SameLine();
            if (empty)
            {
                ImGui::BeginDisabled();
            }
            if (ImGui::Button("Eject"))
            {
                wd_1793::ejectDisk(index);
            }
            if (empty)
            {
                ImGui::EndDisabled();
            }
        }
    }
}