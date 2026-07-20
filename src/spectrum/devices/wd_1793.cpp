#include <fstream>

#include "wd_1793.h"
#include "main.h"

#define TRACKHEADER 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, \
                    0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, \
                    0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, \
                    0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, \
                    0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, \
                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xc2, 0xc2, 0xc2, 0xfc, \
                    0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, \
                    0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, \
                    0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, \
                    0x4e, 0x4e

#define SECTORHEADER_PRE 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xa1, 0xa1, 0xa1, 0xfe, 0x00, 0x00

#define SECTORHEADER_POST 0x01, 0x00, 0x00, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, \
                            0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e

#define SECTORDATA_PRE 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xa1, 0xa1, 0xa1, 0xfb

#define SECTORDATA 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
                    0x00, 0x00
                
#define SECTORDATA_POST 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, \
                        0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, \
                        0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, \
                        0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, \
                        0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e

#define TRACK_POST 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, \
                    0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, \
                    0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, \
                    0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, \
                    0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, \
                    0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, \
                    0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, \
                    0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, \
                    0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, \
                    0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, \
                    0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, \
                    0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, \
                    0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, \
                    0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, \
                    0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e, \
                    0x4e, 0x4e, 0x4e, 0x4e, 0x4e, 0x4e

namespace wd_1793
{
    namespace
    {
        constexpr uint8_t SYSTEM_34_TRACK[] =
        {
            TRACKHEADER,
            SECTORHEADER_PRE, 0x01, SECTORHEADER_POST, SECTORDATA_PRE, SECTORDATA, SECTORDATA_POST,
            SECTORHEADER_PRE, 0x02, SECTORHEADER_POST, SECTORDATA_PRE, SECTORDATA, SECTORDATA_POST,
            SECTORHEADER_PRE, 0x03, SECTORHEADER_POST, SECTORDATA_PRE, SECTORDATA, SECTORDATA_POST,
            SECTORHEADER_PRE, 0x04, SECTORHEADER_POST, SECTORDATA_PRE, SECTORDATA, SECTORDATA_POST,
            SECTORHEADER_PRE, 0x05, SECTORHEADER_POST, SECTORDATA_PRE, SECTORDATA, SECTORDATA_POST,
            SECTORHEADER_PRE, 0x06, SECTORHEADER_POST, SECTORDATA_PRE, SECTORDATA, SECTORDATA_POST,
            SECTORHEADER_PRE, 0x07, SECTORHEADER_POST, SECTORDATA_PRE, SECTORDATA, SECTORDATA_POST,
            SECTORHEADER_PRE, 0x08, SECTORHEADER_POST, SECTORDATA_PRE, SECTORDATA, SECTORDATA_POST,
            SECTORHEADER_PRE, 0x09, SECTORHEADER_POST, SECTORDATA_PRE, SECTORDATA, SECTORDATA_POST,
            SECTORHEADER_PRE, 0x0a, SECTORHEADER_POST, SECTORDATA_PRE, SECTORDATA, SECTORDATA_POST,
            SECTORHEADER_PRE, 0x0b, SECTORHEADER_POST, SECTORDATA_PRE, SECTORDATA, SECTORDATA_POST,
            SECTORHEADER_PRE, 0x0c, SECTORHEADER_POST, SECTORDATA_PRE, SECTORDATA, SECTORDATA_POST,
            SECTORHEADER_PRE, 0x0d, SECTORHEADER_POST, SECTORDATA_PRE, SECTORDATA, SECTORDATA_POST,
            SECTORHEADER_PRE, 0x0e, SECTORHEADER_POST, SECTORDATA_PRE, SECTORDATA, SECTORDATA_POST,
            SECTORHEADER_PRE, 0x0f, SECTORHEADER_POST, SECTORDATA_PRE, SECTORDATA, SECTORDATA_POST,
            SECTORHEADER_PRE, 0x10, SECTORHEADER_POST, SECTORDATA_PRE, SECTORDATA, SECTORDATA_POST,
            TRACK_POST
        };

        constexpr int WD1793_STEP_TACTS = 112;

        constexpr uint32_t RATES[8][4] =
        {
            {1500, 3000, 5000, 7500},
            {750, 1500, 2500, 3750},
            {1500, 3000, 5000, 7500},
            {750, 1500, 2500, 3750},
            {92, 95, 99, 104},
            {46, 47, 49, 52},
            {92, 95, 99, 104},
            {46, 47, 49, 52}
        };

        constexpr uint16_t SECTOR_DATA_POSITION[16] = {162, 554, 946, 1338, 1730, 2122, 2514, 2906, 3298, 3690, 4082, 4474, 4866, 5258, 5650, 6042};

                
        constexpr int DiskControlRead = 0x000;
        constexpr int DiskControlSeekUp = 0x100;
        constexpr int DiskControlSeekDown = 0x300;
        constexpr int DiskControlWrite = 0x400;

        constexpr int DiskOutStepping = 0x40;
        constexpr int DiskOutTrack0 = 0x80;
        constexpr int DiskOutIndex = 0x20;

        constexpr int CLK = 0x1;
        constexpr int DDEN = 0x2;
        constexpr int Test = 0x4;

        constexpr int RateSelect = CLK | DDEN | Test;

        constexpr int HLD = 0x8;
        constexpr int HLT = 0x10;
        constexpr int INTRQ = 0x20;
        constexpr int Dire = 0x40;
        constexpr int Writing = 0x80;
        constexpr int DRQ = 0x100;
        constexpr int CommandType = 0x200;
        constexpr int FINTRQ = 0x400;
        constexpr int ONE = 0x800;
        constexpr int Power0 = 0x1000;
        constexpr int Power1 = 0x2000;
        constexpr int Power2 = 0x4000;
        constexpr int Power3 = 0x8000;
        constexpr int NotToReady = 0x10000;
        constexpr int ReadyToNot = 0x20000;
        constexpr int IndexPulse = 0x40000;
        constexpr int Inmediate = 0x80000;

        constexpr int StepIdle = 0x0;
        constexpr int StepWaiting = 0x1;
        constexpr int StepWaitingMark = 0x2;
        constexpr int StepReadByte = 0x3;
        constexpr int StepWriteByte = 0x4;
        constexpr int StepLastWriteByte = 0x5;
        constexpr int StepWaitIndex = 0x6;
        constexpr int StepWriteRaw = 0x7;

        constexpr int None = 0x0;
        constexpr int SettingHeader = 0x1;
        constexpr int SettingEnd = 0x2;
        constexpr int TypeI0 = 0x3;
        constexpr int TypeI1 = 0x4;
        constexpr int TypeICheck = 0x5;
        constexpr int TypeIUpdate = 0x6;
        constexpr int TypeISeek = 0x7;
        constexpr int TypeIEnd = 0x8;
        constexpr int ReadHeader = 0x9;
        constexpr int TypeIHeaderReaded = 0xa;
        constexpr int ReadHeaderBytes = 0xb;
        constexpr int ReadCRC = 0xc;
        constexpr int TypeIHeadSet = 0xd;
        constexpr int TypeIISetHead = 0xe;
        constexpr int TypeIICommand = 0xf;
        constexpr int ReadDataFlag = 0x10;
        constexpr int ReadDataFlag2 = 0x11;
        constexpr int ReadData = 0x12;
        constexpr int ReadSectorHeader = 0x13;
        constexpr int ReadAddressWait = 0x14;
        constexpr int ReadAddressBytes = 0x15;
        constexpr int WriteDataFlag = 0x16;
        constexpr int WriteData = 0x17;
        constexpr int WriteCRC1 = 0x18;
        constexpr int WriteCRC2 = 0x19;
        constexpr int WriteEnd = 0x1a;
        constexpr int WriteLast = 0x1b;
        constexpr int ReadAddressDataFlag = 0x1c;
        constexpr int WriteTrack = 0x1d;
        constexpr int WriteTrackCRC = 0x1e;
        constexpr int WriteTrackStart = 0x1f;
        constexpr int ReadTrackStart = 0x20;
        constexpr int ReadTrackData = 0x21;

        constexpr int StatusBusy = 0x1;
        constexpr int StatusIndex = 0x2;
        constexpr int StatusTrack0 = 0x4;
        constexpr int StatusCRC = 0x8;
        constexpr int StatusSeek = 0x10;
        constexpr int StatusHeadLoaded = 0x20;
        constexpr int StatusProtected = 0x40;
        constexpr int StatusNotReady = 0x80;

        constexpr int StatusDataRequest = 0x2;
        constexpr int StatusLostData = 0x4;
        constexpr int StatusRecordNotFound = 0x10;
        constexpr int StatusRecordType = 0x20;
        constexpr int StatusWriteFault = 0x20;

        constexpr int StatusSetWP = 0x100;
        constexpr int StatusSetTrack0 = 0x200;
        constexpr int StatusSetIndex = 0x400;
        constexpr int StatusSetHead = 0x800;

        constexpr int HeadBit = 0x8;
        constexpr int UpdateBit = 0x10;
        constexpr int VerifyBit = 0x4;
        constexpr int StepInOut = 0x40;
        constexpr int TypeI = 0x80;

        constexpr int Mark = 0xa1a1a1;
        constexpr int IndexMark = 0xc2;
        constexpr int SectorMark = 0xa1;

        struct Disk
        {
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

        uint32_t state, stepState, nextState;
        uint32_t control;
        uint32_t counter;

        uint8_t command;
        uint8_t track;
        uint8_t sector;
        uint8_t data;
        uint8_t sataSeekRegister;
        uint16_t status;

        uint8_t header[7];
        uint8_t headerIndex;

        uint8_t selectedDiskIndex;
        uint8_t previousDiskIndex;

        uint8_t retries;

        uint64_t markAtHead;
        uint8_t byteAtHead, byteToWrite;

        uint8_t side;

        Disk *disks[4];

        bool fastMode;

        bool sclConverted;
        uint8_t track0[2304];

        int writeTrackMark, writeTrackSector;

        void readDiskData(Disk* disk, uint8_t* buffer, int count)
        {
            while (count--)
            {
                if (disk->dataIndex >= disk->dataLength)
                {
                    *buffer++ = 0;
                }
                else
                {
                    *buffer++ = disk->data[disk->dataIndex++];
                }
            }
        }

        void writeDiskData(Disk* disk, uint8_t* buffer, int count)
        {
            while (count--)
            {
                disk->data[disk->dataIndex++] = *buffer++;
            }
        }

        void SCLtoTRD(Disk* disk, uint8_t* track0)
        {
            uint8_t numberOfFiles;

            memset(track0, 0, 2304);

            disk->dataIndex = 8;
            readDiskData(disk, &numberOfFiles, 1);

            char diskNameArray[9] = "SCL_DISK";

            int startSector = 0;
            int startTrack = 1;

            uint8_t data;
            for (int i = 0; i < numberOfFiles; ++i)
            {
                int n = i << 4;

                for (int j = 0; j < 13; ++j)
                {
                    readDiskData(disk, &data, 1);
                    track0[n + j] = data;
                }

                readDiskData(disk, &data, 1);
                track0[n + 13] = data;
                track0[n + 14] = (uint8_t)startSector;
                track0[n + 15] = (uint8_t)startTrack;

                int newStartTrack = (startTrack * 16 + startSector + data) / 16;
                startSector = (startTrack * 16 + startSector + data) - 16 * newStartTrack;
                startTrack = newStartTrack;
            }

            track0[2273] = (uint8_t)startSector;
            track0[2274] = (uint8_t)startTrack;
            track0[2275] = 22;
            track0[2276] = (uint8_t)numberOfFiles;
            uint16_t freeSectors = 2560 - (startTrack << 4) + startSector;
            track0[2277] = freeSectors & 0x00ff;
            track0[2278] = freeSectors >> 8;
            track0[2279] = 0x10;

            for (int i = 0; i < 9; ++i)
            {
                track0[2282 + i] = 0x20;
            }

            for (int i = 0; i < 8; ++i)
            {
                track0[2293 + i] = diskNameArray[i];
            }

            disk->sclDataOffset = (9 + (numberOfFiles * 14)) - 4096;
        }

        void diskStepRead()
        {
            Disk* disk = disks[selectedDiskIndex];

            disk->byteAtHead = 0;

            disk->signal = disk->trackIndex ? 0 : DiskOutTrack0;

            if (disk->indexDelay)
            {
                --disk->indexDelay;
                disk->signal |= DiskOutIndex;
                return;
            }

            if (disk->sectorBufferIndex < 0xff)
            {
                ++disk->sectorBufferIndex;
                ++disk->index;
                disk->byteAtHead = disk->sectorBuffer[disk->sectorBufferIndex];
                return;
            }

            if (disk->index != 0xffffffff && disk->index >= /*6417*/ 6663)
            {
                disk->index = 0xffffffff;
                disk->sectorBufferIndex = 0xff;
                disk->indexDelay = 25;
                return;
            }

            ++disk->index;

            const uint32_t cursect = (disk->index - 146) / 392;

            if (cursect < 16)
            {
                if (disk->index == SECTOR_DATA_POSITION[cursect])
                {
                    disk->byteAtHead = (!disk->trackIndex && side) ? disk->track0side1data : disk->trackIndex;
                    return;
                }

                if (disk->index == SECTOR_DATA_POSITION[cursect] + 44)
                {
                    if ((disk->scl) && (!disk->trackIndex) && (!side))
                    {
                        if (!sclConverted)
                        {
                            SCLtoTRD(disk, track0);
                            sclConverted = true;
                        }

                        if (cursect < 9)
                        {
                            memcpy(disk->sectorBuffer, track0 + (cursect << 8), 0x100);
                        }
                        else
                        {
                            memset(disk->sectorBuffer, 0, 0x100);
                        }
                    }
                    else
                    {
                        const int seekptr = (disk->trackIndex << (11 + disk->sideCount)) + (side << 12) + (cursect << 8) + disk->sclDataOffset;

                        disk->dataIndex = seekptr;
                        readDiskData(disk, disk->sectorBuffer, 0x100);
                    }

                    disk->byteAtHead = disk->sectorBuffer[0];
                    disk->sectorBufferIndex = 0;

                    return;
                }
            }

            disk->byteAtHead = SYSTEM_34_TRACK[disk->index];
        }

        void diskStepWrite(uint8_t byte)
        {
            Disk *disk = disks[selectedDiskIndex];

            disk->byteAtHead = 0;

            disk->signal = disk->trackIndex ? 0 : DiskOutTrack0;

            if (disk->indexDelay)
            {
                --disk->indexDelay;
                disk->signal |= DiskOutIndex;
                return;
            }

            if (disk->sectorBufferIndex < 0xff)
            {
                ++disk->sectorBufferIndex;

                disk->sectorBuffer[disk->sectorBufferIndex] = byte;

                if (disk->sectorBufferIndex == 0xff)
                {
                    disk->dataIndex -= 0x100;
                    writeDiskData(disk, disk->sectorBuffer, 0x100);
                }

                ++disk->index;

                return;
            }

            if (disk->index != 0xffffffff && disk->index >= 6663)
            {
                disk->index = 0xffffffff;
                disk->sectorBufferIndex = 0xff;
                disk->indexDelay = 25;
                return;
            }

            ++disk->index;

            const uint32_t cursect = (disk->index - 146) / 392;

            if (cursect < 16)
            {
                if (disk->index == SECTOR_DATA_POSITION[cursect] + 44)
                {
                    const int seekptr = (disk->trackIndex << (11 + disk->sideCount)) + (side << 12) + (cursect << 8);

                    disk->dataIndex = seekptr;
                    readDiskData(disk, disk->sectorBuffer, 0x100);

                    disk->sectorBuffer[0] = byte;

                    disk->sectorBufferIndex = 0;
                }
            }
        }
        
        void endProcess()
        {
            status &= ~StatusBusy;
            state = None;
            stepState = StepIdle;
            control &= ~(Writing | DRQ);
            retries = 15;
            led = 0;
            control |= INTRQ;
        }

        void process()
        {
            switch (state)
            {

            case SettingHeader:
            {
                if (control & HLD)
                {
                    state = nextState;
                    process();
                    return;
                }

                led = 1;

                control |= HLD;
                counter = 1;
                state = SettingEnd;
                stepState = StepWaiting;
                return;
            }

            case SettingEnd:
            {
                control |= HLT;
                state = nextState;
                process();
                return;
            }

            case TypeI0:
            {
                if (command & HeadBit)
                {
                    nextState = TypeI1;
                    state = SettingHeader;
                }
                else
                {
                    control &= ~HLD | HLT;
                    state = TypeI1;
                }
                process();
                return;
            }

            case TypeI1:
            {
                if (command & StepInOut)
                {
                    if (command & 0x20)
                    {
                        control &= ~Dire;
                    }
                    else
                    {
                        control |= Dire;
                    }

                    if (command & UpdateBit)
                    {
                        state = TypeIUpdate;
                    }
                    else
                    {
                        state = TypeISeek;
                    }
                }
                else
                {
                    if (command & 0x20)
                    {
                        if (command & UpdateBit)
                        {
                            state = TypeIUpdate;
                        }
                        else
                        {
                            state = TypeISeek;
                        }
                    }
                    else
                    {
                        if (!(command & 0x10))
                        {
                            track = 0xff;
                            data = 0;
                        }

                        sataSeekRegister = data;
                        state = TypeICheck;
                    }
                }

                process();
                return;
            }

            case TypeICheck:
            {
                if (track == sataSeekRegister)
                {
                    state = TypeIEnd;
                }
                else
                {
                    if (sataSeekRegister > track)
                    {
                        control |= Dire;
                    }
                    else
                    {
                        control &= ~Dire;
                    }
                    state = TypeIUpdate;
                }
                process();
                return;
            }

            case TypeIUpdate:
            {
                if (control & Dire)
                {
                    ++track;
                }
                else
                {
                    --track;
                }

                state = TypeISeek;
                process();
                return;
            }

            case TypeISeek:
            {
                if (!(control & Dire) && (disks[selectedDiskIndex]->signal & DiskOutTrack0))
                {
                    track = 0;
                    state = TypeIEnd;
                    process();
                    return;
                }
                else
                {
                    if (control & Dire)
                    {
                        if (disks[selectedDiskIndex]->trackIndex < disks[selectedDiskIndex]->trackCount)
                        {
                            ++disks[selectedDiskIndex]->trackIndex;
                        }
                    }
                    else
                    {
                        if (disks[selectedDiskIndex]->trackIndex > 0)
                        {
                            --disks[selectedDiskIndex]->trackIndex;
                        }
                    }

                    diskStepRead();

                    led = 1;

                    counter = (RATES[(control & RateSelect) ^ 0x4][command & 0x3]) >> 3;

                    stepState = StepWaiting;
                    if (!(command & 0xe0))
                    {
                        state = TypeICheck;
                    }
                    else
                    {
                        state = TypeIEnd;
                    }

                    return;
                }
            }

            case TypeIEnd:
            {
                if (command & VerifyBit)
                {
                    if (control & HLD)
                    {
                        nextState = TypeIHeadSet;
                        state = SettingHeader;
                    }
                    else
                    {
                        retries = 5;
                        state = ReadHeader;
                        nextState = TypeIHeaderReaded;
                    }

                    process();
                    return;
                }
                else
                {
                    control |= INTRQ;
                    status |= StatusSetHead;
                    status &= ~StatusBusy;
                }
                return;
            }

            case TypeIHeadSet:
            {
                retries = 5;
                state = ReadHeader;
                nextState = TypeIHeaderReaded;
                process();
                return;
            }

            case ReadHeader:
            {
                if (!retries)
                {
                    status |= StatusSeek;
                    endProcess();
                    return;
                }

                stepState = StepWaitingMark;
                markAtHead = Mark;

                headerIndex = 0xff;
                state = ReadHeaderBytes;
                return;
            }

            case ReadAddressWait:
            {
                if (!retries)
                {
                    status |= StatusSeek;
                    endProcess();
                    return;
                }

                stepState = StepWaitingMark;
                markAtHead = Mark;

                headerIndex = 0xff;
                state = ReadAddressDataFlag;
                return;
            }

            case ReadAddressDataFlag:
            {
                stepState = StepReadByte;
                state = ReadAddressBytes;
                return;
            }

            case ReadAddressBytes:
            {
                if (!retries)
                {
                    status |= StatusSeek;
                    endProcess();
                    return;
                }

                led = 1;

                if (headerIndex == 0xff)
                {
                    if (byteAtHead != 0xfe)
                    {
                        state = ReadAddressWait;
                        process();
                        return;
                    }
                }
                else
                {
                    if (headerIndex < 7)
                    {
                        header[headerIndex] = byteAtHead;
                    }

                    data = byteAtHead;

                    if (control & DRQ)
                    {
                        status |= StatusLostData;
                    }
                    control |= DRQ;
                }

                ++headerIndex;

                if (headerIndex == 0x6)
                {
                    sector = header[0];
                    endProcess();
                    return;
                }

                return;
            }

            case ReadHeaderBytes:
            {
                if (!retries)
                {
                    status |= StatusSeek;
                    endProcess();
                    return;
                }

                if (headerIndex != 0xff)
                {
                    if (headerIndex < 7)
                    {
                        header[headerIndex] = byteAtHead;
                    }
                }

                ++headerIndex;

                if (headerIndex == 0x7)
                {
                    state = nextState;
                    process();
                    return;
                }

                stepState = StepReadByte;
                return;
            }

            case TypeIHeaderReaded:
            {
                if (!retries)
                {
                    status |= StatusSeek;
                    endProcess();
                    return;
                }

                if (header[0] != 0xfe)
                {
                    state = ReadHeader;
                    process();
                    return;
                }

                if (header[1] != track)
                {
                    state = ReadHeader;
                    process();
                    return;
                }

                status &= ~StatusCRC;
                endProcess();
                return;
            }

            case TypeIISetHead:
            {
                nextState = TypeIICommand;
                state = SettingHeader;
                process();
                return;
            }

            case TypeIICommand:
            {
                if ((command & 0xc0) == 0x80)
                {
                    if (command & 0x20)
                    {
                        if (disks[selectedDiskIndex]->writeProtect)
                        {
                            status |= StatusProtected;
                            endProcess();
                            return;
                        }
                    }

                    retries = 5;
                    stepState = StepWaitingMark;
                    markAtHead = Mark;

                    headerIndex = 0xff;
                    state = ReadHeaderBytes;
                    nextState = ReadSectorHeader;
                }
                else if ((command & 0xf0) == 0xc0)
                {
                    retries = 5;
                    state = ReadAddressWait;
                    process();
                }
                else if ((command & 0xf0) == 0xf0)
                {
                    if (disks[selectedDiskIndex]->writeProtect)
                    {
                        status |= StatusProtected;
                        endProcess();
                        return;
                    }

                    state = WriteTrackStart;
                    stepState = StepWaitIndex;
                    control |= DRQ;
                    writeTrackMark = 0;
                }
                else if ((command & 0xf0) == 0xe0)
                {
                    state = ReadTrackStart;
                    stepState = StepWaitIndex;
                }

                return;
            }

            case ReadSectorHeader:
            {
                if (header[0] != 0xfe)
                {
                    state = ReadHeader;
                    process();
                    return;
                }

                if (header[1] != track)
                {
                    state = ReadHeader;
                    process();
                    return;
                }

                if (!fastMode || command & 0x20)
                {
                    if (header[3] != sector)
                    {
                        state = ReadHeader;
                        process();
                        return;
                    }

                    if ((command & 0x2) && (header[2] & ((command >> 3) & 0x1)))
                    {
                        state = ReadHeader;
                        process();
                        return;
                    }
                }
                else
                {
                    if ((command & 0x2) && (header[2] & ((command >> 3) & 0x1)))
                    {
                        state = ReadHeader;
                        process();
                        return;
                    }

                    header[3] = sector;
                    disks[selectedDiskIndex]->index = SECTOR_DATA_POSITION[sector - 1] + 39;
                }

                status &= ~StatusCRC;

                counter = 0x100;

                stepState = StepWaitingMark;
                markAtHead = Mark;

                if (command & 0x20)
                {
                    state = WriteDataFlag;
                    control |= DRQ;
                }
                else
                {
                    state = ReadDataFlag;
                }

                return;
            }

            case ReadDataFlag:
            {
                stepState = StepReadByte;
                state = ReadDataFlag2;
                return;
            }

            case WriteDataFlag:
            {
                stepState = StepWriteByte;
                state = WriteData;
                control |= Writing;
                byteAtHead = (command & 0x1) ? 0xf8 : 0xfb;
                return;
            }

            case WriteData:
            {
                if (control & DRQ)
                {
                    if (disks[selectedDiskIndex]->sectorBufferIndex < 0xff)
                    {
                        disks[selectedDiskIndex]->dataIndex -= 0x100;
                        writeDiskData(disks[selectedDiskIndex], disks[selectedDiskIndex]->sectorBuffer, disks[selectedDiskIndex]->sectorBufferIndex);
                    }

                    status |= StatusLostData;
                    control &= ~Writing;
                    endProcess();
                    return;
                }

                led = 2;

                byteAtHead = data;
                data = 0;

                if (--counter)
                {
                    control |= DRQ;
                }
                else
                {
                    state = WriteCRC1;
                }
                return;
            }

            case WriteCRC1:
            {
                state = WriteCRC2;
                return;
            }

            case WriteCRC2:
            {
                state = WriteLast;
                return;
            }

            case WriteLast:
            {
                state = WriteEnd;
                stepState = StepLastWriteByte;
                return;
            }

            case WriteEnd:
            {
                control &= ~Writing;

                if (command & 0x10)
                {
                    ++sector;
                    state = TypeIICommand;
                    process();
                }
                else
                {
                    endProcess();
                }
                return;
            }

            case ReadDataFlag2:
            {
                if (byteAtHead == 0xf8)
                {
                    status |= StatusRecordType;
                }
                else if (byteAtHead == 0xfb)
                {
                    status &= ~StatusRecordType;
                }
                else
                {
                    state = TypeIICommand;
                    process();
                    stepState = None;
                    return;
                }
                state = ReadData;
                return;
            }

            case ReadData:
            {
                led = 1;

                data = byteAtHead;

                if (control & DRQ)
                {
                    status |= StatusLostData;
                }

                control |= DRQ;
                if (!--counter)
                {
                    state = ReadCRC;
                    counter = 2;
                    return;
                }
                return;
            }

            case ReadCRC:
            {
                if (!--counter)
                {
                    status &= ~StatusCRC;

                    if (command & 0x10)
                    {
                        ++sector;

                        if (!fastMode || sector > 16)
                        {
                            state = TypeIICommand;
                            process();
                        }
                        else
                        {
                            header[3] = sector;
                            disks[selectedDiskIndex]->index = SECTOR_DATA_POSITION[sector - 1] + 39;

                            status &= ~StatusCRC;
                            counter = 0x100;

                            stepState = StepWaitingMark;
                            markAtHead = Mark;

                            state = ReadDataFlag;
                        }
                    }
                    else
                    {
                        endProcess();
                    }
                }

                return;
            }

            case WriteTrackStart:
            {
                if (control & DRQ)
                {
                    status |= StatusLostData;
                    endProcess();
                    return;
                }

                control |= Writing;
                state = WriteTrack;

                disks[selectedDiskIndex]->index = 0xffffffff;
                disks[selectedDiskIndex]->sectorBufferIndex = 0xff;
                disks[selectedDiskIndex]->indexDelay = 0;

                process();
                return;
            }

            case WriteTrack:
            {

                if (!retries)
                {
                    endProcess();
                    return;
                }

                if (control & DRQ)
                {
                    if (disks[selectedDiskIndex]->sectorBufferIndex < 0xff)
                    {
                        disks[selectedDiskIndex]->dataIndex -= 0x100;
                        writeDiskData(disks[selectedDiskIndex], disks[selectedDiskIndex]->sectorBuffer, disks[selectedDiskIndex]->sectorBufferIndex);
                    }

                    status |= StatusLostData;
                    control &= ~Writing;
                    endProcess();
                    return;
                }

                led = 2;

                switch (data)
                {

                case 0xf5:
                {
                    ++writeTrackMark;

                    byteAtHead = SectorMark;
                    stepState = StepWriteByte;
                    control |= DRQ;
                    break;
                }

                case 0xf7:
                {
                    writeTrackMark = 0;

                    byteAtHead = 0;
                    stepState = StepWriteByte;
                    state = WriteTrackCRC;
                    break;
                }

                default:
                {
                    if (writeTrackMark == 3 && data == 0xfe)
                    {
                        writeTrackMark = 0b100000000;
                    }
                    else if (writeTrackMark == 3 && data == 0xfb)
                    {
                        disks[selectedDiskIndex]->index = SECTOR_DATA_POSITION[writeTrackSector - 1] + 41;
                    }
                    else if (writeTrackMark & 0b100000000)
                    {
                        ++writeTrackMark;
                        if (writeTrackMark == 0b100000001)
                        {
                            if (track == 0 && side == 1)
                            {
                                disks[selectedDiskIndex]->track0side1data = data;
                            }
                        }
                        else
                        {
                            if (writeTrackMark == 0b100000011)
                            {
                                writeTrackSector = data;
                                writeTrackMark = 0;
                            }
                        }
                    }
                    else
                    {
                        writeTrackMark = 0;
                    }

                    byteAtHead = data;
                    stepState = StepWriteByte;
                    control |= DRQ;
                    break;
                }
                }

                return;
            }

            case WriteTrackCRC:
            {
                byteAtHead = 0x0;
                stepState = StepWriteByte;
                state = WriteTrack;
                control |= DRQ;
                return;
            }

            case ReadTrackStart:
            {
                stepState = StepReadByte;
                state = ReadTrackData;
                retries = 1;
                return;
            }

            case ReadTrackData:
            {
                if (!retries)
                {
                    endProcess();
                    return;
                }

                led = 1;

                if (control & DRQ)
                {
                    status |= StatusLostData;
                }

                control |= DRQ;
                data = byteAtHead;
                return;
            }
            }
        }
    }

    uint8_t led;

    void ioWrite(uint16_t port, uint8_t value)
    {
        switch (port & 0x00ff)
        {

        case 0x1f:

            if ((value & 0xf0) == 0xd0)
            {
                if (status & StatusBusy)
                {
                    status &= ~StatusBusy;
                    state = None;
                    stepState = StepIdle;
                    control &= ~(Writing | DRQ);
                    retries = 15;
                    led = 0;
                }
                else
                {
                    status = StatusSetIndex | StatusSetTrack0 | StatusSetWP;
                }

                if ((value & 0xf) == 0x0)
                {
                    control &= ~(INTRQ | FINTRQ);
                }
                else
                {
                    if (value & 0x8)
                    {
                        control |= FINTRQ;
                    }
                    else
                    {
                        control = (control & 0xffff) | ((value & 0xf) << 16); // Set conditions
                    }
                }

                return;
            }

            if (!(status & StatusBusy))
            {
                control &= ~(INTRQ | FINTRQ);

                command = value;

                if (disks[selectedDiskIndex] && (control & (Power0 << selectedDiskIndex)))
                {
                    if (command & TypeI)
                    {
                        if ((command & 0xc0) == 0x80 || (command & 0xfb) == 0xc0 || (command & 0xfb) == 0xf0 || (command & 0xfb) == 0xe0)
                        {
                            state = TypeIISetHead;
                            status = StatusBusy;

                            if (command & VerifyBit)
                            {
                                stepState = StepWaiting;
                                counter = 937;
                            }
                            else
                            {
                                process();
                            }
                        }
                        else
                        {
                            control |= INTRQ;
                        }
                    }
                    else
                    {
                        status = StatusSetIndex | StatusSetTrack0 | StatusSetWP | StatusBusy;
                        state = TypeI0;
                        process();
                    }
                }
                else
                {
                    status = StatusNotReady;
                    control |= INTRQ;
                }
            }

            break;

        case 0x3f:
            track = value;
            break;

        case 0x5f:
            sector = value;
            break;

        case 0x7f:
            data = value;
            control &= ~DRQ;
            break;

        case 0x00ff:
            if (selectedDiskIndex != (value & 0x3))
            {
                selectedDiskIndex = value & 0x3;
                if (disks[selectedDiskIndex] != NULL && side && disks[selectedDiskIndex]->sideCount == 1)
                {
                    side = 0;
                }
                sclConverted = false;
            }

            if (!(value & 0x4))
            {
                reset();
            }

            if (value & 0x8)
            {
                control |= Test;
            }
            else
            {
                control &= ~Test;
            }

            if (value & 0x10)
            {
                side = 0;
            }
            else
            {
                if (disks[selectedDiskIndex] != NULL)
                {
                    side = disks[selectedDiskIndex]->sideCount == 1 ? 0 : 1;
                }
                else
                {
                    side = 1;
                }
            }

            if (value & 0x40)
            {
                control |= DDEN;
            }
            else
            {
                control &= ~DDEN;
            }
                
            break;
        }
    }

    uint8_t ioRead(uint16_t port)
    {
        uint8_t r;

        switch (port & 0x00ff)
        {

        case 0x1f:
            control &= ~INTRQ;

            r = status & 0xff;

            if (disks[selectedDiskIndex])
            {
                if (status & StatusSetWP)
                {
                    if (disks[selectedDiskIndex]->writeProtect)
                    {
                        r |= StatusProtected;
                    }
                    else
                    {
                        r &= ~StatusProtected;
                    }
                }

                if ((status & StatusSetTrack0) && (disks[selectedDiskIndex]->signal & DiskOutTrack0))
                {
                    r |= StatusTrack0;
                }

                if ((status & StatusSetIndex) && (disks[selectedDiskIndex]->signal & DiskOutIndex))
                {
                    r |= StatusIndex;
                }
                else
                {
                    if (control & DRQ)
                        r |= StatusDataRequest;
                }

                if (status & StatusSetHead)
                {
                    r |= StatusHeadLoaded;
                }
            }
            else
            {
                r |= StatusNotReady;
            }

            return r;

        case 0x3f:
            return track;

        case 0x5f:
            return sector;

        case 0x7f:
            control &= ~DRQ;
            return data;

        case 0x00ff:
        {
            uint8_t v = 0;
            if (control & DRQ) v |= 0x40;
            if (control & (INTRQ | FINTRQ)) v |= 0x80;
            return v;
        }

        }

        return 0;
    }

    void tact()
    {
        if ((main::currentTact % WD1793_STEP_TACTS) != 0 || (control & (HLD | HLT) == 0))
        {
            return;
        }

        uint8_t diskMark = 0x00;
        uint8_t signal = 0x00;

        if (disks[selectedDiskIndex])
        {
            if ((control & Writing) && !disks[selectedDiskIndex]->writeProtect)
            {
                diskStepWrite(byteToWrite);
                diskMark = 0;
            }
            else
            {
                diskStepRead();
                diskMark = disks[selectedDiskIndex]->byteAtHead;
            }

            signal = disks[selectedDiskIndex]->signal;
        }

        uint8_t diskIndex = signal ^ previousDiskIndex;
        previousDiskIndex = signal;

        switch (stepState)
        {

        case StepIdle:
        {
            if (retries && (diskIndex & DiskOutIndex) && (signal & DiskOutIndex))
            {
                --retries;
                if (!retries)
                {
                    control &= ~(HLD | HLT);
                    return;
                }
            }
            break;
        }

        case StepWaiting:
        {
            if (fastMode)
            {
                counter = 0;
                process();
            }
            else if (!(--counter))
            {
                process();
            }

            break;
        }

        case StepWaitingMark:
        {
            if (retries && (diskIndex & DiskOutIndex) && (signal & DiskOutIndex))
            {
                --retries;
                if (!retries)
                {
                    process();
                }
            }

            if ((markAtHead & 0xff) == diskMark)
            {
                markAtHead >>= 8;
                if (!markAtHead)
                {
                    process();
                    if (control & Writing)
                        goto write;
                }
            }
            else
            {
                markAtHead = Mark;
            }

            break;
        }

        case StepReadByte:
        {
            if (retries && (diskIndex & DiskOutIndex) && (signal & DiskOutIndex))
            {
                --retries;
                if (!retries)
                    process();
            }

            byteAtHead = diskMark;
            process();
            byteAtHead = 0;

            break;
        }

        case StepWriteByte:
        {
        write:
            if (retries && (diskIndex & DiskOutIndex) && (signal & DiskOutIndex))
            {
                --retries;
                if (!retries)
                {
                    process();
                }
            }

            byteToWrite = byteAtHead;
            process();

            break;
        }

        case StepWriteRaw:
        {
            if (retries && (diskIndex & DiskOutIndex) && (signal & DiskOutIndex))
            {
                --retries;
                if (!retries)
                {
                    process();
                }
            }

            byteToWrite = byteAtHead;
            process();

            break;
        }

        case StepLastWriteByte:
            process();
            break;

        case StepWaitIndex:
        {
            if ((diskIndex & DiskOutIndex) && (signal & DiskOutIndex))
            {
                retries = 1;
                process();
            }
            break;
        }

        }
    }

    void reset()
    {
        state = None;
        stepState = StepIdle;
        nextState = None;
        counter = 0;
        control &= 0xf000;
        command = sector = data = sataSeekRegister = 0x0;
        status = StatusSetIndex | StatusSetTrack0 | StatusSetWP;
        track = 0xff;
        writeTrackMark = 0;
        headerIndex = 0;
        retries = 0;
        side = 0;
        selectedDiskIndex = 0;
        fastMode = false;
        sclConverted = false;
    }

    bool insertDisk(uint8_t unit, std::string fileName)
    {
        ejectDisk(unit);

        disks[unit] = new Disk();

        std::ifstream file(fileName, std::ios::in | std::ios::binary);
        if (!file.is_open())
        {
            return false;
        }

        file.seekg(0, std::ios::end);
        std::streampos fileSize = file.tellg();
        disks[unit]->dataLength = (int)fileSize;
        disks[unit]->data = new uint8_t[disks[unit]->dataLength];

        file.seekg(0, std::ios::beg);
        file.read((char *)disks[unit]->data, fileSize);
        file.close();

        uint8_t diskType;

        std::string magic = std::string((char *)(disks[unit]->data), 8);

        if (magic == "SINCLAIR")
        {
            disks[unit]->scl = true;
            disks[unit]->writeProtect = true;
            diskType = 0x16;
        }
        else
        {
            disks[unit]->scl = false;
            disks[unit]->writeProtect = false;
            disks[unit]->sclDataOffset = 0;
            diskType = disks[unit]->data[2048 + 227];
        }

        switch (diskType)
        {
        case 0x16:
            disks[unit]->trackCount = 79;
            disks[unit]->sideCount = 2;
            break;
        case 0x17:
            disks[unit]->trackCount = 39;
            disks[unit]->sideCount = 2;
            break;
        case 0x18:
            disks[unit]->trackCount = 79;
            disks[unit]->sideCount = 1;
            break;
        case 0x19:
            disks[unit]->trackCount = 39;
            disks[unit]->sideCount = 1;
            break;
        default:
            ejectDisk(unit);
            return false;
        }

        if (!disks[unit]->scl)
        {
            if (disks[unit]->dataLength > disks[unit]->sideCount * disks[unit]->trackCount * 16 * 256)
            {
                int i;
                for (int i = disks[unit]->trackCount + 1; i < 83; ++i)
                {
                    if (disks[unit]->sideCount * i * 16 * 256 >= disks[unit]->dataLength)
                    {
                        disks[unit]->trackCount = i;
                        break;
                    }
                }
            }
        }

        disks[unit]->dataIndex = 0;

        disks[unit]->track0side1data = 0;
        disks[unit]->sectorBufferIndex = 0xff;

        control |= Power0 << unit;

        return true;
    }

    void ejectDisk(uint8_t unit)
    {
        if (disks[unit] != NULL)
        {
            delete[disks[unit]->dataLength] disks[unit]->data;
            disks[unit] = NULL;

            if (selectedDiskIndex == unit)
            {
                endProcess();

                side = 0;

                sclConverted = false;
            }

            control &= ~(Power0 << unit);
        }
    }
}
