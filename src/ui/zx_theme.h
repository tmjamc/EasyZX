#pragma once

#include "imgui.h"

namespace ImGui
{
    constexpr float ZX_LABEL_WIDTH = 150.0f;

    enum ZXIconId { FOLDER, TAPE, DISK, DEFAULT, HDD, USB, UP, REFRESH };

    void ZXTheme();
    void ZXThemeCleanUp();
    bool ZXBegin(const char* name, ImGuiWindowFlags flags);
    void ZXLabel(const char* name, float width = ZX_LABEL_WIDTH);
    void ZXIcon(ZXIconId id);
    bool ZXButtonIcon(ZXIconId id, const char* name);
    bool ZXCollapsingHeader(const char* name, bool &collapsed);
}