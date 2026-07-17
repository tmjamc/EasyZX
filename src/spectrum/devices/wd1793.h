#pragma once

#include <cstdint>
#include <string>

namespace wd1793
{
    extern uint8_t led;

    void ioWrite(uint16_t port,uint8_t value);
    uint8_t ioRead(uint16_t port);
    void tact();
    void reset();
    bool insertDisk(uint8_t unit, std::string fileName);
    void ejectDisk(uint8_t unit);
}