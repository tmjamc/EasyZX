#include "imgui.h"
#include "settings.h"
#include "giga_screen.h"

namespace widgets
{
    namespace
    {
        constexpr const char* GIGA_SCREEN_MODE[3] = { "Off", "On", "Automatic"};
    }

    void renderGigaScreen()
    {
        ImGui::AlignTextToFramePadding();
        ImGui::TextUnformatted("Giga screen:");

        ImGui::SameLine();
        if (ImGui::BeginCombo("##giga_screen", GIGA_SCREEN_MODE[settings::current.displayGigaScreenMode]))
        {
            for (int index = 0; index < 3; ++index)
            {
                const bool is_selected = (index == settings::current.displayGigaScreenMode);
                if (ImGui::Selectable(GIGA_SCREEN_MODE[index], is_selected))
                {
                    if (index != settings::current.displayGigaScreenMode)
                    {
                        settings::current.displayGigaScreenMode = index;
                        settings::save();
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