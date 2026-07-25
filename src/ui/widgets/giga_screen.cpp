#include "zx_theme.h"
#include "settings.h"
#include "giga_screen.h"
#include "ula.h"

namespace widgets
{
    namespace
    {
        constexpr const char* GIGA_SCREEN_MODE[3] = { "Off", "On", "Automatic"};
    }

    void renderGigaScreen()
    {
        ImGui::ZXLabel("Giga screen:");

        ImGui::SameLine();

        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        ImVec2 posMin = ImGui::GetCursorScreenPos();
        posMin.x -= 40.0f;
        posMin.y += 10.0f;
        ImVec2 posMax = ImVec2(posMin.x + 30.0f, posMin.y + 12.0f);
        draw_list->AddRectFilled(posMin, posMax, ula::gigaScreen ? 0xff00ff00 : ImGui::GetColorU32(ImGuiCol_Button), 3.0f);

        ImGui::SetNextItemWidth(-1.0f);
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