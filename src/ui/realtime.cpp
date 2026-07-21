#include "realtime.h"
#include "imgui.h"

namespace realtime
{
    namespace
    {

    }

    bool opened = true;

    void render()
    {
        ImGui::SetNextWindowSize(ImVec2(300, 80));
        if (ImGui::Begin("Realtime status###Test2", &opened, ImGuiWindowFlags_NoCollapse))
        {
            // ImGui::AlignTextToFramePadding();
            ImGui::TextUnformatted("Model:");
        }
        ImGui::End();
    }
}