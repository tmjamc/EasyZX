#include <filesystem>

#include "tape_manager.h"
#include "tape.h"
#include "zx_theme.h"

namespace widgets
{
    namespace
    {
        bool collapsed = true;
    }
    
    void renderTape()
    {
        if (ImGui::ZXCollapsingHeader("Tape", collapsed))
        {
            const bool empty = tape::fileName == nullptr;

            // ImGui::Dummy(ImVec2(0.0f, 0.0f));
            // ImDrawList* drawList = ImGui::GetWindowDrawList();
            // ImVec2 posMin = ImGui::GetCursorScreenPos();
            // const ImVec2 posMax = ImVec2(posMin.x + 30.0f, posMin.y + 12.0f);
            // drawList->AddRectFilled(posMin, posMax, 0xff00ff00, 3.0f);
            ImGui::AlignTextToFramePadding();

            // ImGui::SameLine();
            // ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 30.0f);

            const float width = ImGui::GetContentRegionAvail().x - 131.0f;
            if (empty)
            {
                ImGui::ZXLabel("No tape", width);
            }
            else
            {
                std::filesystem::path tapePath(tape::fileName);
                ImGui::ZXLabel(tapePath.filename().generic_string().c_str(), width);
            }

            ImGui::SameLine();
            if (ImGui::Button("Insert", ImVec2(60.0f, 0.0f)))
            {
                // TODO: Show disk browse dialog
            }

            ImGui::SameLine();
            if (empty)
            {
                ImGui::BeginDisabled();
            }
            if (ImGui::Button("Eject", ImVec2(60.0f, 0.0f)))
            {
                // wd_1793::ejectDisk(index);
            }
            if (empty)
            {
                ImGui::EndDisabled();
            }
        }

    }
}