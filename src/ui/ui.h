#pragma once

#include "imgui.h"

namespace ui
{
    constexpr float LABEL_WIDTH = 150.0f;

    enum IconId { FOLDER, TAPE, DISK, DEFAULT, HDD, USB, UP, REFRESH };

    void init();
    void cleanUp();
    void render();
    bool Begin(const char* name, ImGuiWindowFlags flags);
    void Label(const char* name, float width = LABEL_WIDTH);
    void Icon(IconId id);
    bool ButtonIcon(IconId id, const char* name);
    bool CollapsingHeader(const char* name, bool &collapsed);
}