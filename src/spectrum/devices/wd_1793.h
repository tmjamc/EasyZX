#pragma once

#include <cstdint>
#include <string>

namespace wd_1793
{
    struct Disk
    {
        std::string fileName;
        uint16_t trackCount;
        uint8_t sideCount;
        uint8_t byteAtHead;
        uint8_t signal;
        uint32_t trackIndex;
        uint32_t index;
        uint32_t indexDelay;
        bool writeProtect;
        uint8_t sectorBuffer[0x100];
        uint16_t sectorBufferIndex;
        uint8_t *data;
        int dataLength;
        int dataIndex;
        bool scl;
        int sclDataOffset;
        int track0side1data;
    };

    extern uint8_t led[4];
    extern bool enabled;
    extern Disk* disks[4];

    void ioWrite(uint16_t port,uint8_t value);
    uint8_t ioRead(uint16_t port);
    void tact();
    void reset();
    void cleanUp();
    bool insertDisk(uint8_t unit, std::string fileName);
    void ejectDisk(uint8_t unit);
}