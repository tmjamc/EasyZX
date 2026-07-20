#pragma once

#include <main.h>

namespace ula
{
    extern uint8_t portData;

    void init();
    void cleanUp();
    void tact();
    void contendedTacts(uint16_t addr, int tacts, bool force = false);
    void ioPreTacts(uint16_t port);
    void ioPostTacts(uint16_t port);
    uint8_t readPort(uint16_t port);
    uint8_t readBus();
}