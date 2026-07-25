#include <format>
#include <filesystem>

#include "zx_theme.h"
#include "disk_drive.h"
#include "wd_1793.h"

namespace widgets
{
    void renderDiskDrive()
    {
        if (!wd_1793::enabled)
        {
            ImGui::BeginDisabled();
        }

        ImDrawList* draw_list = ImGui::GetWindowDrawList();

        for (int index = 0; index < 4; ++index)
        {
            const bool empty = wd_1793::disks[index] == nullptr;

            ImGui::ZXLabel(std::format("Unit {}", index).c_str(), 50.0f);

            ImGui::SameLine();

            ImVec2 posMin = ImGui::GetCursorScreenPos();
            posMin.y += 10.0f;
            ImVec2 posMax = ImVec2(posMin.x + 30.0f, posMin.y + 12.0f);
            draw_list->AddRectFilled(posMin, posMax, wd_1793::led[index] == 0 ? ImGui::GetColorU32(ImGuiCol_Button) : (wd_1793::led[index] == 1 ? 0xff00ff00 : 0xff0000ff), 3.0f);
            
            ImGui::SameLine();
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 30.0f);
            ImGui::SetNextItemWidth(ImGui::GetWindowWidth() - 253.0f);
            if (empty)
            {
                ImGui::LabelText("", "No disk");
            }
            else
            {
                std::filesystem::path diskPath(wd_1793::disks[index]->fileName);
                ImGui::LabelText("", diskPath.filename().generic_string().c_str());
            }

            ImGui::SameLine();
            if (ImGui::Button("Insert", ImVec2(60.0f, 0.0f)))
            {
                // Show disk browse dialog
            }

            ImGui::SameLine();
            if (empty && wd_1793::enabled)
            {
                ImGui::BeginDisabled();
            }
            if (ImGui::Button("Eject", ImVec2(60.0f, 0.0f)))
            {
                wd_1793::ejectDisk(index);
            }
            if (empty && wd_1793::enabled)
            {
                ImGui::EndDisabled();
            }
        }

        if (!wd_1793::enabled)
        {
            ImGui::EndDisabled();
        }

    }
}