#include <filesystem>

#include "tape_manager.h"
#include "tape.h"
#include "zx_theme.h"
#include "settings.h"
#include "file_browser.h"

namespace widgets
{
    namespace
    {
        constexpr const char* TAPE_SPEED[3] = { "Normal speed", "Throttle", "Instant"};
    }
    
    void renderTapeManager()
    {
        bool empty = tape::fileName == nullptr;

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
            file_browser::open("C:\\Users\\jam\\Documents\\Projects\\EasyZX_Deploy", file_browser::tapeFilters);
        }

        ImGui::SameLine();
        ImGui::BeginDisabled(empty);
        if (ImGui::Button("Eject", ImVec2(60.0f, 0.0f)))
        {
            tape::reset();
            tape::cleanUp();
            empty = true;
        }

        ImGui::ZXLabel("Current block:");

        ImGui::SameLine();
        ImGui::SetNextItemWidth(-1.0f);
        if (ImGui::BeginCombo("##block", empty ? "" : tape::blocks[tape::blockIndex].getInfo().c_str()))
        {
            if (!empty)
            {
                for (int index = 0; index < tape::blocks.size(); ++index)
                {
                    const bool is_selected = (index == tape::blockIndex);
                    if (ImGui::Selectable(std::format("{}###{}", tape::blocks[index].getInfo(), index).c_str(), is_selected))
                    {
                        if (index != tape::blockIndex)
                        {
                            tape::playing = 0;
                            tape::blockIndex = index;
                            tape::endOfTape = false;
                        }
                    }
                    if (is_selected)
                    {
                        ImGui::SetItemDefaultFocus();
                    }
                }
            }
                
            ImGui::EndCombo();
        }

        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 151.0f);
        ImGui::ProgressBar(empty ? 0.0f : tape::getBlockProgress(), ImVec2(ImGui::GetContentRegionAvail().x - 1.0f, 6.0f), "");

        ImGui::EndDisabled();

        ImGui::ZXLabel("Auto play / stop:");

        ImGui::SameLine();
        ImGui::Checkbox("", &settings::current.tapeAutoStartStop);

        ImGui::BeginDisabled(empty);

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
            tape::stop();
        }
        ImGui::EndDisabled();

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