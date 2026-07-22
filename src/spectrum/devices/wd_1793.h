#pragma once

#include <cstdint>
#include <string>

namespace wd_1793
{
    extern uint8_t led;
    extern bool enabled;

    void ioWrite(uint16_t port,uint8_t value);
    uint8_t ioRead(uint16_t port);
    void tact();
    void reset();
    void cleanUp();
    bool insertDisk(uint8_t unit, std::string fileName);
    void ejectDisk(uint8_t unit);
}