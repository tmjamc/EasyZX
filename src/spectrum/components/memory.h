#pragma once

namespace memory
{
    extern uint8_t** banks;
    extern uint8_t** ramPages;
    extern uint8_t** romPages;
    extern int activeRamPage;
    extern int activeRomPage;
    extern int activeScreenPage;

    void reset();
    void cleanUp();
    void setPaging(uint8_t data);
    uint8_t read(uint16_t addr);
    void write(uint16_t addr, uint8_t data);
}