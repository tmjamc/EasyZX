#pragma once

#include <cstdint>
#include <string>

namespace wd1793
{
    extern uint8_t led;

    void rvmWD1793Write(uint16_t addr,uint8_t value);
    uint8_t rvmWD1793Read(uint16_t addr);
    void rvmWD1793Step();
    void rvmWD1793Reset();
    bool rvmWD1793InsertDisk(unsigned char UnitNum, std::string Filename);
    void wdDiskEject(unsigned char UnitNum);
}