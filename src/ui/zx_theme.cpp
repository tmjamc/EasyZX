#include "zx_theme.h"

namespace ImGui
{
    bool ZXBegin(const char* name, bool* p_open, ImGuiWindowFlags flags)
    {
        flags |= ImGuiWindowFlags_NoTitleBar;
        if (!ImGui::Begin(name, p_open, flags))
        {
            return false;
        }

        // Custom title bar
        const float titleBarHeight = 30.0f;

        ImVec2 windowPos = ImGui::GetWindowPos();
        ImVec2 windowSize = ImGui::GetWindowSize();

        ImDrawList* drawList = ImGui::GetWindowDrawList();

        // Background
        drawList->AddRectFilled(windowPos, ImVec2(windowPos.x + windowSize.x, windowPos.y + titleBarHeight), IM_COL32_BLACK);
        
        // Title text
        // const char* nameEnd = ImGui::FindRenderedTextEnd(name);
        drawList->AddText(ImVec2(windowPos.x + 10, windowPos.y + 7), IM_COL32_WHITE, name);
        
        // // Drag region
        // ImGui::SetCursorScreenPos(windowPos);
        // ImGui::InvisibleButton("##titlebar", ImVec2(windowSize.x, titleBarHeight));
        
        // if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left))
        // {
        //     ImGui::SetWindowPos(ImVec2(windowPos.x + ImGui::GetIO().MouseDelta.x, windowPos.y + ImGui::GetIO().MouseDelta.y));
        // }
        
        ImGui::SetCursorPosY(titleBarHeight + 5);

        return true;
    }
}