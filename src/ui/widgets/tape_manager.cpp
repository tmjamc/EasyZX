#include <filesystem>

#include "tape_manager.h"
#include "tape.h"
#include "zx_theme.h"
#include "settings.h"

namespace widgets
{
    namespace
    {
        constexpr const char* TAPE_SPEED[3] = { "Normal speed", "Throttle", "Instant"};

        bool collapsed = true;
    }
    
    void renderTape()
    {
        if (ImGui::ZXCollapsingHeader("Tape", collapsed))
        {
            const bool empty = tape::fileName == nullptr;

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

            ImGui::ZXLabel("Current block:");

            ImGui::SameLine();
            ImGui::SetNextItemWidth(-1.0f);
            if (ImGui::BeginCombo("##block", tape::blocks[tape::blockIndex].getInfo().c_str()))
            {
                for (int index = 0; index < tape::blocks.size(); ++index)
                {
                    const bool is_selected = (index == tape::blockIndex);
                    if (ImGui::Selectable(tape::blocks[index].getInfo().c_str(), is_selected))
                    {
                        if (index != tape::blockIndex)
                        {
                            tape::playing = 0;
                            tape::blockIndex = index;
                        }
                    }
                    if (is_selected)
                    {
                        ImGui::SetItemDefaultFocus();
                    }
                }

                ImGui::EndCombo();
            }

            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 151.0f);
            ImGui::ProgressBar(tape::getBlockProgress(), ImVec2(ImGui::GetContentRegionAvail().x - 1.0f, 6.0f), "");

            ImGui::ZXLabel("Auto play / stop:");

            ImGui::SameLine();
            ImGui::Checkbox("", &settings::current.tapeAutoStartStop);

            ImGui::SameLine();
            ImGui::BeginDisabled(tape::playing);
            if (ImGui::Button("Play", ImVec2(60.0f, 0.0f)))
            {
                settings::current.tapeAutoStartStop = false;
                tape::play();
            }
            ImGui::EndDisabled();
            
            ImGui::SameLine();
            ImGui::BeginDisabled(!tape::playing);
            if (ImGui::Button("Stop", ImVec2(60.0f, 0.0f)))
            {
                settings::current.tapeAutoStartStop = false;
                tape::playing = 0;
            }
            ImGui::EndDisabled();

            ImGui::SameLine();
            ImGui::SetNextItemWidth(-1.0f);
            const int selectedIndex = settings::current.tapeThrottleLoading ? 1 : (settings::current.tapeInstantLoading ? 2 : 0);
            if (ImGui::BeginCombo("##speed", TAPE_SPEED[selectedIndex]))
            {
                for (int index = 0; index < 3; ++index)
                {
                    const bool is_selected = (index == selectedIndex);
                    if (ImGui::Selectable(TAPE_SPEED[index], is_selected))
                    {
                        switch (index)
                        {
                        case 0:
                            settings::current.tapeThrottleLoading = false;
                            settings::current.tapeInstantLoading = false;
                            break;
                        case 1:
                            settings::current.tapeThrottleLoading = true;
                            settings::current.tapeInstantLoading = false;
                            break;
                        case 2:
                            settings::current.tapeThrottleLoading = false;
                            settings::current.tapeInstantLoading = true;
                            break;
                        }
                    }
                    if (is_selected)
                    {
                        ImGui::SetItemDefaultFocus();
                    }
                }

                ImGui::EndCombo();
            }
        }
    }
}