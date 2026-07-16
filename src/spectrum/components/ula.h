#pragma once

#include <main.h>

namespace ula
{
    constexpr static uint32_t PALETTE[16] =
    {
        0x000000,   // black
        0x0000c0,   // blue
        0xc00000,   // red
        0xc000c0,   // magenta
        0x00c000,   // green
        0x00c0c0,   // cyan
        0xc0c000,   // yellow
        0xc0c0c0,   // white
        0x000000,   // bright black
        0x0000ff,   // bright blue
        0xff0000,   // bright red
        0xff00ff,   // bright magenta
        0x00ff00,   // bright green
        0x00ffff,   // bright cyan
        0xffff00,   // bright yellow
        0xffffff    // bright white
    };


    void init(main::Model model);
    void cleanUp();
    void tack();
}