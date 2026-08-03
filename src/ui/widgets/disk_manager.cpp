#include <format>
#include <filesystem>

#include "zx_theme.h"
#include "disk_manager.h"
#include "wd_1793.h"
#include "file_browser.h"

namespace widgets
{
    namespace
    {
        uint8_t unit;

        void loadDiskFile(const std::string &fileName)
        {
            wd_1793::insertDisk(unit, fileName);
        }
    }

    void renderDiskManager()
    {        
        ImGui::BeginDisabled(!wd_1793::enabled);

        ImDrawList* drawList = ImGui::GetWindowDrawList();

        for (int index = 0; index < 4; ++index)
        {
            const bool empty = wd_1793::disks[index] == nullptr;

            ImGui::ZXLabel(std::format("Unit {}", index).c_str(), 50.0f);

            ImGui::SameLine();

            ImVec2 posMin = ImGui::GetCursorScreenPos();
            posMin.y += 10.0f;
            const ImVec2 posMax = ImVec2(posMin.x + 30.0f, posMin.y + 12.0f);
            drawList->AddRectFilled(posMin, posMax, wd_1793::led[index] == 0 ? ImGui::GetColorU32(ImGuiCol_Button) : (wd_1793::led[index] == 1 ? 0xff00ff00 : 0xff0000ff), 3.0f);
            
            ImGui::SameLine();
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 30.0f);
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - 141.0f);
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
            if (ImGui::Button(std::format("Insert###insert_unit_{}", index).c_str(), ImVec2(60.0f, 0.0f)))
            {
                unit = index;
                file_browser::open(file_browser::diskFilters, loadDiskFile);
            }

            ImGui::SameLine();
            ImGui::BeginDisabled(empty && wd_1793::enabled);
            if (ImGui::Button(std::format("Eject###eject_unit_{}", index).c_str(), ImVec2(60.0f, 0.0f)))
            {
                wd_1793::ejectDisk(index);
            }
            ImGui::EndDisabled();
        }

        ImGui::EndDisabled();
    }
}