#pragma once

#include "imgui.h"

namespace ImGui
{
    constexpr float ZX_LABEL_WIDTH = 150.0f;

    void ZXTheme();
    bool ZXBegin(const char* name, bool* p_open, ImGuiWindowFlags flags);
    void ZXLabel(const char* name, float width = ZX_LABEL_WIDTH);
}