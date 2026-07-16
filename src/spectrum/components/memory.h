#pragma once

#include "main.h"

namespace memory
{
    void cleanUp();
    void init(main::Model model);
    void setPaging(uint8_t data);
    uint8_t read(uint16_t addr);
    void write(uint16_t addr, uint8_t data);
}