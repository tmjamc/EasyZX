#include "zx_theme.h"

namespace ImGui
{
    namespace
    {
        constexpr float SPECTRUM_BAR_HEIGHT = 22.0f;
    }

    void ZXTheme()
    {
        ImGuiStyle* style = &ImGui::GetStyle();
        style->WindowBorderSize = 2.0f;
        style->FrameBorderSize = 0.0f;
        style->ScrollbarRounding = 0.0f;
        style->ScrollbarPadding = 0.0f;
        style->ScrollbarSize = 16.0f;
        style->Colors[ImGuiCol_TitleBgCollapsed] = ColorConvertU32ToFloat4(IM_COL32_BLACK);
        style->Colors[ImGuiCol_TitleBgActive] = ColorConvertU32ToFloat4(IM_COL32_BLACK);
        style->Colors[ImGuiCol_TitleBg] = ColorConvertU32ToFloat4(IM_COL32_BLACK);
        style->Colors[ImGuiCol_Border] = ColorConvertU32ToFloat4(IM_COL32_BLACK);
        style->Colors[ImGuiCol_BorderShadow] = ColorConvertU32ToFloat4(IM_COL32_BLACK);
        style->Colors[ImGuiCol_ScrollbarBg] = ColorConvertU32ToFloat4(IM_COL32_BLACK);
        style->Colors[ImGuiCol_ScrollbarGrab] = ColorConvertU32ToFloat4(0xffc0c000);
        style->Colors[ImGuiCol_ScrollbarGrabActive] = ColorConvertU32ToFloat4(0xffffff00);
        style->Colors[ImGuiCol_ScrollbarGrabHovered] = ColorConvertU32ToFloat4(0xffffff00);
        style->Colors[ImGuiCol_Text] = ColorConvertU32ToFloat4(IM_COL32_BLACK);
        style->Colors[ImGuiCol_WindowBg] = ColorConvertU32ToFloat4(IM_COL32_WHITE);
    }

    bool ZXBegin(const char* name, bool* p_open, ImGuiWindowFlags flags)
    {
        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32_WHITE);
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4.0f, 4.0f));
        const bool result = ImGui::Begin(name, p_open, flags);
        ImGui::PopStyleVar();
        ImGui::PopStyleColor();

        ImVec2 windowPos = ImGui::GetWindowPos();
        ImVec2 windowSize = ImGui::GetWindowSize();
        ImDrawList* drawList = ImGui::GetWindowDrawList();

        drawList->PushClipRectFullScreen();

        windowPos.x -= 16.0f;
        windowPos.y -= 1.0f;
        drawList->AddQuadFilled(ImVec2(windowPos.x + windowSize.x - SPECTRUM_BAR_HEIGHT * 2, windowPos.y + SPECTRUM_BAR_HEIGHT), ImVec2(windowPos.x + windowSize.x - SPECTRUM_BAR_HEIGHT, windowPos.y), ImVec2(windowPos.x + windowSize.x, windowPos.y), ImVec2(windowPos.x + windowSize.x - SPECTRUM_BAR_HEIGHT, windowPos.y + SPECTRUM_BAR_HEIGHT), 0xffffff00);
        
        windowPos.x -= SPECTRUM_BAR_HEIGHT - 0.5f;
        drawList->AddQuadFilled(ImVec2(windowPos.x + windowSize.x - SPECTRUM_BAR_HEIGHT * 2, windowPos.y + SPECTRUM_BAR_HEIGHT), ImVec2(windowPos.x + windowSize.x - SPECTRUM_BAR_HEIGHT, windowPos.y), ImVec2(windowPos.x + windowSize.x, windowPos.y), ImVec2(windowPos.x + windowSize.x - SPECTRUM_BAR_HEIGHT, windowPos.y + SPECTRUM_BAR_HEIGHT), 0xff00ff00);
        
        windowPos.x -= SPECTRUM_BAR_HEIGHT - 0.5f;
        drawList->AddQuadFilled(ImVec2(windowPos.x + windowSize.x - SPECTRUM_BAR_HEIGHT * 2, windowPos.y + SPECTRUM_BAR_HEIGHT), ImVec2(windowPos.x + windowSize.x - SPECTRUM_BAR_HEIGHT, windowPos.y), ImVec2(windowPos.x + windowSize.x, windowPos.y), ImVec2(windowPos.x + windowSize.x - SPECTRUM_BAR_HEIGHT, windowPos.y + SPECTRUM_BAR_HEIGHT), 0xff00ffff);
        
        windowPos.x -= SPECTRUM_BAR_HEIGHT - 0.5f;
        drawList->AddQuadFilled(ImVec2(windowPos.x + windowSize.x - SPECTRUM_BAR_HEIGHT * 2, windowPos.y + SPECTRUM_BAR_HEIGHT), ImVec2(windowPos.x + windowSize.x - SPECTRUM_BAR_HEIGHT, windowPos.y), ImVec2(windowPos.x + windowSize.x, windowPos.y), ImVec2(windowPos.x + windowSize.x - SPECTRUM_BAR_HEIGHT, windowPos.y + SPECTRUM_BAR_HEIGHT), 0xff0000ff);

        drawList->PopClipRect();

        // // Custom title bar
        // const float titleBarHeight = 24.0f;


        // ImDrawList* drawList = ImGui::GetWindowDrawList();

        // // Background
        // drawList->AddRectFilled(windowPos, ImVec2(windowPos.x + windowSize.x, windowPos.y + titleBarHeight), IM_COL32_BLACK);
        
        // // Title text
        // // const char* nameEnd = ImGui::FindRenderedTextEnd(name);
        // drawList->AddText(ImVec2(windowPos.x + 6, windowPos.y + 6), IM_COL32_WHITE, name);
        
        // // Drag region
        // ImGui::SetCursorScreenPos(windowPos);
        // ImGui::InvisibleButton("##titlebar", ImVec2(windowSize.x, titleBarHeight));
        
        // if (ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left))
        // {
        //     ImGui::SetWindowPos(ImVec2(windowPos.x + ImGui::GetIO().MouseDelta.x, windowPos.y + ImGui::GetIO().MouseDelta.y));
        // }
        
        // ImGui::SetCursorPosY(titleBarHeight + 5);

        return result;
    }
}