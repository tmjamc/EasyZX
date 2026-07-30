#pragma once

#include "imgui.h"

namespace ImGui
{
    constexpr float ZX_LABEL_WIDTH = 150.0f;

    enum ZXIconId { DEFAULT, FOLDER, TAPE };

    void ZXTheme();
    void ZXThemeCleanUp();
    bool ZXBegin(const char* name, ImGuiWindowFlags flags);
    void ZXLabel(const char* name, float width = ZX_LABEL_WIDTH);
    void ZXIcon(ZXIconId id);
    bool ZXCollapsingHeader(const char* name, bool &collapsed);
}