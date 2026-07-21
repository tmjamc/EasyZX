#pragma once

#include <main.h>

namespace ula
{
    extern uint8_t portData;
    extern bool gigaScreen;

    void init();
    void cleanUp();
    void tact();
    void updateDisplayBuffer();
    void contendedTacts(uint16_t addr, int tacts, bool force = false);
    void ioPreTacts(uint16_t port);
    void ioPostTacts(uint16_t port);
    uint8_t readPort(uint16_t port);
    uint8_t readBus();
}