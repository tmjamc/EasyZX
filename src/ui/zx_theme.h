#pragma once

#include "imgui.h"

namespace ImGui
{
    void ZXTheme();
    bool ZXBegin(const char* name, bool* p_open, ImGuiWindowFlags flags);
}