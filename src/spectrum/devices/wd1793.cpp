#include <fstream>

#include "wd1793.h"
#include "main.h"

namespace wd1793
{
    #define kRVMwdDiskControlRead 0x000
    #define kRVMwdDiskControlSeekUp 0x100
    #define kRVMwdDiskControlSeekDown 0x300
    #define kRVMwdDiskControlWrite 0x400

    #define kRVMwdDiskOutStepping 0x40
    #define kRVMwdDiskOutTrack0 0x80
    #define kRVMwdDiskOutIndex 0x20

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
                       0x00, 0x00 // Sector Data CRC
                    
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

    static constexpr unsigned char SYSTEM_34_TRACK[] =
    {
        // Track 0, Side 0
        TRACKHEADER,
        // Sector 1
        SECTORHEADER_PRE, 0x01, SECTORHEADER_POST, SECTORDATA_PRE, SECTORDATA, SECTORDATA_POST,
        // Sector 2
        SECTORHEADER_PRE, 0x02, SECTORHEADER_POST, SECTORDATA_PRE, SECTORDATA, SECTORDATA_POST,
        // Sector 3
        SECTORHEADER_PRE, 0x03, SECTORHEADER_POST, SECTORDATA_PRE, SECTORDATA, SECTORDATA_POST,
        // Sector 4
        SECTORHEADER_PRE, 0x04, SECTORHEADER_POST, SECTORDATA_PRE, SECTORDATA, SECTORDATA_POST,
        // Sector 5
        SECTORHEADER_PRE, 0x05, SECTORHEADER_POST, SECTORDATA_PRE, SECTORDATA, SECTORDATA_POST,
        // Sector 6
        SECTORHEADER_PRE, 0x06, SECTORHEADER_POST, SECTORDATA_PRE, SECTORDATA, SECTORDATA_POST,
        // Sector 7
        SECTORHEADER_PRE, 0x07, SECTORHEADER_POST, SECTORDATA_PRE, SECTORDATA, SECTORDATA_POST,
        // Sector 8
        SECTORHEADER_PRE, 0x08, SECTORHEADER_POST, SECTORDATA_PRE, SECTORDATA, SECTORDATA_POST,
        // Sector 9
        SECTORHEADER_PRE, 0x09, SECTORHEADER_POST, SECTORDATA_PRE, SECTORDATA, SECTORDATA_POST,
        // Sector 10
        SECTORHEADER_PRE, 0x0a, SECTORHEADER_POST, SECTORDATA_PRE, SECTORDATA, SECTORDATA_POST,
        // Sector 11
        SECTORHEADER_PRE, 0x0b, SECTORHEADER_POST, SECTORDATA_PRE, SECTORDATA, SECTORDATA_POST,
        // Sector 12
        SECTORHEADER_PRE, 0x0c, SECTORHEADER_POST, SECTORDATA_PRE, SECTORDATA, SECTORDATA_POST,
        // Sector 13
        SECTORHEADER_PRE, 0x0d, SECTORHEADER_POST, SECTORDATA_PRE, SECTORDATA, SECTORDATA_POST,
        // Sector 14
        SECTORHEADER_PRE, 0x0e, SECTORHEADER_POST, SECTORDATA_PRE, SECTORDATA, SECTORDATA_POST,
        // Sector 15
        SECTORHEADER_PRE, 0x0f, SECTORHEADER_POST, SECTORDATA_PRE, SECTORDATA, SECTORDATA_POST,
        // Sector 16
        SECTORHEADER_PRE, 0x10, SECTORHEADER_POST, SECTORDATA_PRE, SECTORDATA, SECTORDATA_POST,
        TRACK_POST
    };

    struct rvmwdDisk
    {
        uint16_t trackCount;
        uint8_t sideCount;
        uint8_t a, s;
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

    #define WD177XSTEPSTATES 112 // 112 states -> 14 states per bit

    #define kRVMWD177XCLK 0x1  // 0- 1 mhz, 1- 2mhz
    #define kRVMWD177XDDEN 0x2 // 0- FM, 1- MFM
    #define kRVMWD177XTest 0x4

    #define kRVMWD177XRateSelect kRVMWD177XCLK | kRVMWD177XDDEN | kRVMWD177XTest

    #define kRVMWD177XHLD 0x8  // HEAD LOAD -> Signal HLD commands the drive to load the read/write heads
    #define kRVMWD177XHLT 0x10 // HEAD LOAD TIMING -> Signal HLT informs the controller chip that the head has been properly loaded and to commence read or write operations.
    #define kRVMWD177XINTRQ 0x20
    #define kRVMWD177XDire 0x40
    #define kRVMWD177XWriting 0x80
    #define kRVMWD177XDRQ 0x100
    #define kRVMWD177XCommandType 0x200
    #define kRVMWD177XFINTRQ 0x400
    #define kRVMWD177XONE 0x800
    #define kRVMWD177XPower0 0x1000
    #define kRVMWD177XPower1 0x2000
    #define kRVMWD177XPower2 0x4000
    #define kRVMWD177XPower3 0x8000
    #define kRVMWD177XNotToReady 0x10000
    #define kRVMWD177XReadyToNot 0x20000
    #define kRVMWD177XIndexPulse 0x40000
    #define kRVMWD177XInmediate 0x80000

    // States (Step)
    #define kRVMWD177XStepIdle 0x0
    #define kRVMWD177XStepWaiting 0x1
    #define kRVMWD177XStepWaitingMark 0x2
    #define kRVMWD177XStepReadByte 0x3
    #define kRVMWD177XStepWriteByte 0x4
    #define kRVMWD177XStepLastWriteByte 0x5
    #define kRVMWD177XStepWaitIndex 0x6
    #define kRVMWD177XStepWriteRaw 0x7

    // States
    #define kRVMWD177XNone 0x0
    #define kRVMWD177XSettingHeader 0x1
    #define kRVMWD177XSettingEnd 0x2
    #define kRVMWD177XTypeI0 0x3
    #define kRVMWD177XTypeI1 0x4
    #define kRVMWD177XTypeICheck 0x5
    #define kRVMWD177XTypeIUpdate 0x6
    #define kRVMWD177XTypeISeek 0x7
    #define kRVMWD177XTypeIEnd 0x8
    #define kRVMWD177XReadHeader 0x9
    #define kRVMWD177XTypeIHeaderReaded 0xA
    #define kRVMWD177XReadHeaderBytes 0xB
    #define kRVMWD177XReadCRC 0xC
    #define kRVMWD177XTypeIHeadSet 0xD
    #define kRVMWD177XTypeIISetHead 0xE
    #define kRVMWD177XTypeIICommand 0xF
    #define kRVMWD177XReadDataFlag 0x10
    #define kRVMWD177XReadDataFlag2 0x11
    #define kRVMWD177XReadData 0x12
    #define kRVMWD177XReadSectorHeader 0x13
    #define kRVMWD177XReadAddressWait 0x14
    #define kRVMWD177XReadAddressBytes 0x15
    #define kRVMWD177XWriteDataFlag 0x16
    #define kRVMWD177XWriteData 0x17
    #define kRVMWD177XWriteCRC1 0x18
    #define kRVMWD177XWriteCRC2 0x19
    #define kRVMWD177XWriteEnd 0x1A
    #define kRVMWD177XWriteLast 0x1B
    #define kRVMWD177XReadAddressDataFlag 0x1C
    #define kRVMWD177XWriteTrack 0x1D
    #define kRVMWD177XWriteTrackCRC 0x1E
    #define kRVMWD177XWriteTrackStart 0x1F
    #define kRVMWD177XReadTrackStart 0x20
    #define kRVMWD177XReadTrackData 0x21

    #define kRVMWD177XSettingHeaderTime 3750

    // Status Codes

    #define kRVMWD177XStatusBusy 0x1
    #define kRVMWD177XStatusIndex 0x2
    #define kRVMWD177XStatusTrack0 0x4
    #define kRVMWD177XStatusCRC 0x8
    #define kRVMWD177XStatusSeek 0x10
    #define kRVMWD177XStatusHeadLoaded 0x20
    #define kRVMWD177XStatusProtected 0x40
    #define kRVMWD177XStatusNotReady 0x80

    #define kRVMWD177XStatusDataRequest 0x2
    #define kRVMWD177XStatusLostData 0x4
    #define kRVMWD177XStatusRecordNotFound 0x10
    #define kRVMWD177XStatusRecordType 0x20
    #define kRVMWD177XStatusWriteFault 0x20

    #define kRVMWD177XStatusSetWP 0x100
    #define kRVMWD177XStatusSetTrack0 0x200
    #define kRVMWD177XStatusSetIndex 0x400
    #define kRVMWD177XStatusSetHead 0x800

    // Command bits
    #define kRVMWD177XHeadBit 0x8
    #define kRVMWD177XUpdateBit 0x10
    #define kRVMWD177XVerifyBit 0x4
    #define kRVMWD177XStepInOut 0x40
    #define kRVMWD177XTypeI 0x80

    static constexpr uint32_t srate[8][4] =
    {
        {1500, 3000, 5000, 7500}, // 1mhz mfm test
        {750, 1500, 2500, 3750},  // 2mhz mfm test
        {1500, 3000, 5000, 7500}, // 1mhz fm test
        {750, 1500, 2500, 3750},  // 2mhz fm test
        {92, 95, 99, 104},        // 1mhz mfm !test
        {46, 47, 49, 52},         // 2mhz mfm !test
        {92, 95, 99, 104},        // 1mhz fm !test
        {46, 47, 49, 52},         // 2mhz fm !test
    };

    // Sectdatapos = (cursect * 392) + 146 + 16;
    static constexpr uint16_t SECTOR_DATA_POSITION[16] = {162, 554, 946, 1338, 1730, 2122, 2514, 2906, 3298, 3690, 4082, 4474, 4866, 5258, 5650, 6042};

    #define mark 0xa1a1a1
    #define indexMark 0xC2
    #define sectorMark 0xA1

    uint32_t state, stepState, nextState;
    uint32_t control;
    uint32_t counter;

    uint8_t command;
    uint8_t track;
    uint8_t sector;
    uint8_t data, dsr;
    uint16_t status;

    uint8_t header[7];
    uint8_t headerI;

    uint8_t selectedDiskIndex; // Disk selected
    uint8_t previousDiskIndex; // Disk previous

    uint8_t retry;

    uint64_t marka;
    uint8_t a, e, wb;

    uint16_t crc, aa;
    uint8_t side;

    rvmwdDisk *disks[4];

    bool fastmode;

    bool sclConverted;
    unsigned char track0[2304]; // Store SCL translated track0

    int wtrackmark, wtracksector;

    uint8_t led;

    static void readDiskData(rvmwdDisk *d, unsigned char *buffer, int count)
    {
        while (count--)
        {
            if (d->dataIndex >= d->dataLength)
            {
                *buffer++ = 0;
            }
            else
            {
                *buffer++ = d->data[d->dataIndex++];
            }
        }
    }

    static void writeDiskData(rvmwdDisk *d, unsigned char *buffer, int count)
    {
        while (count--)
        {
            d->data[d->dataIndex++] = *buffer++;
        }
    }

    static void SCLtoTRD(rvmwdDisk *d, unsigned char *track0)
    {

        uint8_t numberOfFiles;

        // Reset track 0 info
        memset(track0, 0, 2304);

        d->dataIndex = 8;
        // fseek(d->Diskfile, 8, SEEK_SET);
        readDiskData(d, &numberOfFiles, 1);
        // fread(&numberOfFiles, 1, 1, d->Diskfile);

        // printf("Number of files: %d\n",(int)numberOfFiles);

        char diskNameArray[9] = "SCL_DISK";

        // Populate FAT.
        int startSector = 0;
        int startTrack = 1; // Since Track 0 is reserved for FAT and Disk Specification.

        uint8_t data;
        for (int i = 0; i < numberOfFiles; i++)
        {

            int n = i << 4;

            for (int j = 0; j < 13; j++)
            {
                readDiskData(d, &data, 1);
                // fread(&data, 1, 1, d->Diskfile);
                track0[n + j] = data;
            }

            readDiskData(d, &data, 1);
            // fread(&data, 1, 1, d->Diskfile); // Filelenght
            track0[n + 13] = data;

            // printf("File #: %d, Filelenght: %d\n",i + 1,(int)data);

            track0[n + 14] = (uint8_t)startSector;
            track0[n + 15] = (uint8_t)startTrack;

            int newStartTrack = (startTrack * 16 + startSector + data) / 16;
            startSector = (startTrack * 16 + startSector + data) - 16 * newStartTrack;
            startTrack = newStartTrack;
        }

        // Populate Disk Specification.
        track0[2273] = (uint8_t)startSector;
        track0[2274] = (uint8_t)startTrack;
        track0[2275] = 22;                     // Disk Type
        track0[2276] = (uint8_t)numberOfFiles; // File Count
        uint16_t freeSectors = 2560 - (startTrack << 4) + startSector;
        // printf("Free Sectors: %d\n",freeSectors);
        track0[2277] = freeSectors & 0x00ff;
        track0[2278] = freeSectors >> 8;
        track0[2279] = 0x10; // TR-DOS ID

        for (int i = 0; i < 9; i++)
            track0[2282 + i] = 0x20;

        // Store the image file name in the disk label section of the Disk Specification.
        for (int i = 0; i < 8; i++)
            track0[2293 + i] = diskNameArray[i];

        d->sclDataOffset = (9 + (numberOfFiles * 14)) - 4096;
    }

    static void rvmwdDiskStep_read()
    {

        rvmwdDisk* disk = disks[selectedDiskIndex];

        disk->a = 0;

        disk->s = disk->trackIndex ? 0 : kRVMwdDiskOutTrack0;

        if (disk->indexDelay)
        {
            disk->indexDelay--;
            disk->s |= kRVMwdDiskOutIndex;
            return;
        }

        if (disk->sectorBufferIndex < 0xff)
        {
            disk->sectorBufferIndex++;
            disk->index++;
            disk->a = disk->sectorBuffer[disk->sectorBufferIndex];
            return;
        }

        if (disk->index != 0xffffffff && disk->index >= /*6417*/ 6663)
        {
            disk->index = 0xffffffff;
            disk->sectorBufferIndex = 0xff;
            disk->indexDelay = 25;
            return;
        }

        disk->index++;

        const uint32_t cursect = (disk->index - 146) / 392;

        if (cursect < 16)
        {

            if (disk->index == SECTOR_DATA_POSITION[cursect])
            { // Track in sector header
                disk->a = (!disk->trackIndex && side) ? disk->track0side1data : disk->trackIndex;
                return;
            }

            if (disk->index == SECTOR_DATA_POSITION[cursect] + 44)
            { // Sector data

                if ((disk->scl) && (!disk->trackIndex) && (!side))
                {

                    // Create track0 from SCL file if not already done
                    if (!sclConverted)
                    {
                        SCLtoTRD(disk, track0);
                        sclConverted = true;
                    }

                    // SCL disk -> Read sector to cache from created Track0
                    if (cursect < 9)
                        memcpy(disk->sectorBuffer, track0 + (cursect << 8), 0x100);
                    else
                        memset(disk->sectorBuffer, 0, 0x100);
                }
                else
                {

                    const int seekptr = (disk->trackIndex << (11 + disk->sideCount)) + (side << 12) + (cursect << 8) + disk->sclDataOffset;

                    disk->dataIndex = seekptr;
                    // fseek(disk->Diskfile, seekptr, SEEK_SET);
                    readDiskData(disk, disk->sectorBuffer, 0x100);
                    // fread(disk->cursectbuf, 0x100, 1, disk->Diskfile);
                }

                disk->a = disk->sectorBuffer[0];
                disk->sectorBufferIndex = 0;

                return;
            }
        }

        disk->a = SYSTEM_34_TRACK[disk->index];
    }

    static void rvmwdDiskStep_write(uint8_t wbyte)
    {

        rvmwdDisk *disk = disks[selectedDiskIndex];

        disk->a = 0;

        disk->s = disk->trackIndex ? 0 : kRVMwdDiskOutTrack0;

        if (disk->indexDelay)
        {
            disk->indexDelay--;
            disk->s |= kRVMwdDiskOutIndex;
            return;
        }

        if (disk->sectorBufferIndex < 0xff)
        {

            disk->sectorBufferIndex++;

            disk->sectorBuffer[disk->sectorBufferIndex] = wbyte;

            // Flush byte to disk
            // fwrite(&wbyte,1,1,disk->Diskfile);

            // Sector complete (flush to disk)
            if (disk->sectorBufferIndex == 0xff)
            {
                disk->dataIndex -= 0x100;                 // fseek(disk->Diskfile, -0x100, SEEK_CUR);
                writeDiskData(disk, disk->sectorBuffer, 0x100); // fwrite(disk->cursectbuf,1,0x100,disk->Diskfile);
            }

            disk->index++;

            return;
        }

        if (disk->index != 0xffffffff && disk->index >= /*6417*/ 6663)
        {
            disk->index = 0xffffffff;
            disk->sectorBufferIndex = 0xff;
            disk->indexDelay = 25;
            return;
        }

        disk->index++;

        const uint32_t cursect = (disk->index - 146) / 392;

        if (cursect < 16)
        {

            // if (disk->indx == sectdatapos[cursect]) { // Track in sector header

            //   return;

            // }

            if (disk->index == SECTOR_DATA_POSITION[cursect] + 44)
            { // Sector data

                const int seekptr = (disk->trackIndex << (11 + disk->sideCount)) + (side << 12) + (cursect << 8);

                disk->dataIndex = seekptr;               // fseek(disk->Diskfile, seekptr, SEEK_SET);
                readDiskData(disk, disk->sectorBuffer, 0x100); // fread(disk->cursectbuf, 0x100, 1, disk->Diskfile);

                // fseek(disk->Diskfile,-0x100,SEEK_CUR); // fseek(disk->Diskfile,seekptr,SEEK_SET); <- What's faster ? SEEK_SET or SEEK_CUR with negative offset ?
                // fwrite(&wbyte,1,1,disk->Diskfile);

                disk->sectorBuffer[0] = wbyte;

                disk->sectorBufferIndex = 0;
            }
        }
    }
    
    static void _end()
    {
        status &= ~kRVMWD177XStatusBusy;
        state = kRVMWD177XNone;
        stepState = kRVMWD177XStepIdle;
        control &= ~(kRVMWD177XWriting | kRVMWD177XDRQ);
        retry = 15;
        led = 0;
        control |= kRVMWD177XINTRQ; // TODO: ADD A INTERRUPT HANDLER
    }

    static void _do()
    {

        switch (state)
        {

        case kRVMWD177XSettingHeader:
        {

            if (control & kRVMWD177XHLD)
            { // Head load
                state = nextState;
                _do();
                return;
            }

            // RVM plays motor audio sample here
            led = 1;

            control |= kRVMWD177XHLD;
            // c=kRVMWD177XSettingHeaderTime * ((control&kRVMWD177XTest)?1:0);
            counter = 1;
            state = kRVMWD177XSettingEnd;
            stepState = kRVMWD177XStepWaiting;
            return;
        }

        case kRVMWD177XSettingEnd:
        {
            control |= kRVMWD177XHLT;
            state = nextState;
            _do();
            return;
        }

        case kRVMWD177XTypeI0:
        {
            if (command & kRVMWD177XHeadBit)
            {
                nextState = kRVMWD177XTypeI1;
                state = kRVMWD177XSettingHeader;
            }
            else
            {
                control &= ~kRVMWD177XHLD | kRVMWD177XHLT;
                state = kRVMWD177XTypeI1;
            }
            _do();
            return;
        }

        case kRVMWD177XTypeI1:
        {
            if (command & kRVMWD177XStepInOut)
            {
                // StepIn or stepOut
                if (command & 0x20)
                {
                    // Step Out
                    // printf("Step Out\n");
                    control &= ~kRVMWD177XDire;
                }
                else
                {
                    // Step In
                    // printf("Step In\n");
                    control |= kRVMWD177XDire;
                }
                if (command & kRVMWD177XUpdateBit)
                {
                    state = kRVMWD177XTypeIUpdate;
                }
                else
                {
                    state = kRVMWD177XTypeISeek;
                }
            }
            else
            {
                if (command & 0x20)
                { // Step
                    if (command & kRVMWD177XUpdateBit)
                    {
                        state = kRVMWD177XTypeIUpdate;
                    }
                    else
                    {
                        state = kRVMWD177XTypeISeek;
                    }
                }
                else
                {
                    if (command & 0x10)
                    { // Seek
                    }
                    else
                    { // Restore
                        track = 0xff;
                        data = 0;
                    }

                    // printf("Seeking disk %d to track %d (disk in track: %d)\n",diskS,data,disk[diskS]->t);
                    dsr = data;
                    state = kRVMWD177XTypeICheck;
                }
            }

            _do();
            return;
        }

        case kRVMWD177XTypeICheck:
        {
            if (track == dsr)
            {
                state = kRVMWD177XTypeIEnd;
            }
            else
            {
                if (dsr > track)
                {
                    control |= kRVMWD177XDire;
                }
                else
                {
                    control &= ~kRVMWD177XDire;
                }
                state = kRVMWD177XTypeIUpdate;
            }
            _do();
            return;
        }

        case kRVMWD177XTypeIUpdate:
        {

            if (control & kRVMWD177XDire)
                track++;
            else
                track--;

            // printf("UPDATE TRACK %d\n",track);
            state = kRVMWD177XTypeISeek;
            _do();
            return;
        }

        case kRVMWD177XTypeISeek:
        {

            if (!(control & kRVMWD177XDire) && (disks[selectedDiskIndex]->s & kRVMwdDiskOutTrack0))
            {

                // printf("No seek track 0 end\n");
                track = 0;
                state = kRVMWD177XTypeIEnd;
                _do();
                return;
            }
            else
            {

                // printf("Seek track: %d side: %d\n", track, side);

                // Seek forward or backward
                // { printf("--- Disk Seek Command ---\n");
                // printf("Current track: %d\n", (int) disk->t);

                if (control & kRVMWD177XDire)
                {
                    if (disks[selectedDiskIndex]->trackIndex < disks[selectedDiskIndex]->trackCount)
                        disks[selectedDiskIndex]->trackIndex++;
                }
                else
                {
                    if (disks[selectedDiskIndex]->trackIndex > 0)
                        disks[selectedDiskIndex]->trackIndex--;
                }

                // printf("New track: %d\n", (int) disk->t);
                // printf("-------------------------\n"); }

                rvmwdDiskStep_read();

                led = 1; // RVM plays seek audio sample here

                counter = (srate[(control & kRVMWD177XRateSelect) ^ 0x4][command & 0x3]) >> 3; // Value for 1 bit per diskstep / 8

                // printf("c: %d, RateSelect: %d\n",c,(control & kRVMWD177XRateSelect) ^ 0x4);
                // printf("RATE: %llu\n",c);
                // printf("STEP\n");

                stepState = kRVMWD177XStepWaiting;
                if (!(command & 0xe0)) // Seek or restore
                    state = kRVMWD177XTypeICheck;
                else
                    state = kRVMWD177XTypeIEnd;

                return;
            }
        }

        case kRVMWD177XTypeIEnd:
        {
            if (command & kRVMWD177XVerifyBit)
            {
                // printf("Verify\n");
                if (control & kRVMWD177XHLD)
                {
                    nextState = kRVMWD177XTypeIHeadSet;
                    state = kRVMWD177XSettingHeader;
                }
                else
                {
                    retry = 5; // 5 retrys
                    state = kRVMWD177XReadHeader;
                    nextState = kRVMWD177XTypeIHeaderReaded;
                }

                _do();
                return;
            }
            else
            {
                // printf("Type I end\n");
                control |= kRVMWD177XINTRQ;
                status |= kRVMWD177XStatusSetHead;
                status &= ~kRVMWD177XStatusBusy;
            }
            return;
        }

        case kRVMWD177XTypeIHeadSet:
        {
            retry = 5; // 5 retrys
            state = kRVMWD177XReadHeader;
            nextState = kRVMWD177XTypeIHeaderReaded;
            _do();
            return;
        }

        case kRVMWD177XReadHeader:
        {

            if (!retry)
            {
                status |= kRVMWD177XStatusSeek;
                _end();
                return;
            }

            // _waitMark();
            // printf("Waitmark ReadHeader\n");
            stepState = kRVMWD177XStepWaitingMark;
            marka = mark;
            //

            headerI = 0xff;
            state = kRVMWD177XReadHeaderBytes;
            return;
        }

        case kRVMWD177XReadAddressWait:
        {

            if (!retry)
            {
                status |= kRVMWD177XStatusSeek;
                _end();
                return;
            }

            //_waitMark();
            // printf("Waitmark ReadAddressWait\n");
            stepState = kRVMWD177XStepWaitingMark;
            marka = mark;
            //

            headerI = 0xff;
            state = kRVMWD177XReadAddressDataFlag;
            return;
        }

        case kRVMWD177XReadAddressDataFlag:
        {
            stepState = kRVMWD177XStepReadByte;
            state = kRVMWD177XReadAddressBytes;
            return;
        }

        case kRVMWD177XReadAddressBytes:
        {

            if (!retry)
            {
                status |= kRVMWD177XStatusSeek;
                _end();
                return;
            }

            led = 1;

            if (headerI == 0xff)
            {
                if (a != 0xfe)
                {
                    state = kRVMWD177XReadAddressWait;
                    _do();
                    return;
                }
            }
            else
            {
                // printf("Data %02x crc:%04x\n",a,crc);
                //  if(headerI == 0) {
                //    header[0]=1;
                //    data=1;
                //  } else {
                if (headerI < 7)
                    header[headerI] = a;
                data = a;
                // }
                if (control & kRVMWD177XDRQ)
                {
                    status |= kRVMWD177XStatusLostData;
                }
                control |= kRVMWD177XDRQ;
            }

            headerI++;

            if (headerI == 0x6)
            {

                sector = header[0];

                // printf("Read Adress sector: %d\n",header[0]);

                // printf("Header crc: %04x\n",crc);

                // if(crc) {
                // status|=kRVMWD177XStatusCRC;
                // }

                _end();

                return;
            }

            return;
        }

        case kRVMWD177XReadHeaderBytes:
        {

            if (!retry)
            {
                status |= kRVMWD177XStatusSeek;
                _end();
                return;
            }

            if (headerI != 0xff)
            {
                if (headerI < 7)
                    header[headerI] = a;
            }

            headerI++;

            if (headerI == 0x7)
            {
                state = nextState;
                _do();
                return;
            }

            stepState = kRVMWD177XStepReadByte;
            return;
        }

        case kRVMWD177XTypeIHeaderReaded:
        {
            if (!retry)
            {
                status |= kRVMWD177XStatusSeek;
                _end();
                return;
            }

            if (header[0] != 0xfe)
            {
                state = kRVMWD177XReadHeader;
                _do();
                return;
            }

            if (header[1] != track)
            {
                state = kRVMWD177XReadHeader;
                _do();
                return;
            }

            // printf("Header readed, track:%d sector:%d\n",header[1],header[3]);

            // if(crc) {
            //   status|=kRVMWD177XStatusCRC;
            // } else {
            status &= ~kRVMWD177XStatusCRC;
            _end();
            return;
            // }
        }

        case kRVMWD177XTypeIISetHead:
        {
            nextState = kRVMWD177XTypeIICommand;
            state = kRVMWD177XSettingHeader;
            _do();
            return;
        }

        case kRVMWD177XTypeIICommand:
        {

            if ((command & 0xc0) == 0x80)
            { // Read or Write Sector

                if (command & 0x20)
                { // Write Sector
                    if (disks[selectedDiskIndex]->writeProtect)
                    {
                        status |= kRVMWD177XStatusProtected;
                        _end();
                        return;
                    }
                    // printf("WRITE SECTOR: track: %d, sector: %d\n",track,sector);
                }
                else
                {
                    // printf("READ SECTOR: track: %d, sector: %d\n",track,sector);
                }
                retry = 5; // 5 retrys

                // state=kRVMWD177XReadHeader;
                // next=kRVMWD177XReadSectorHeader;
                // _do();

                // _waitMark();
                // printf("Waitmark ReadHeader\n");
                stepState = kRVMWD177XStepWaitingMark;
                marka = mark;
                //

                headerI = 0xff;
                state = kRVMWD177XReadHeaderBytes;
                nextState = kRVMWD177XReadSectorHeader;
            }
            else if ((command & 0xf0) == 0xc0)
            { // Read Address

                retry = 5; // 5 retrys
                state = kRVMWD177XReadAddressWait;
                // printf("State -> ReadAddressWait\n");
                _do();
            }
            else if ((command & 0xf0) == 0xf0)
            { // Write Track

                if (disks[selectedDiskIndex]->writeProtect)
                {
                    status |= kRVMWD177XStatusProtected;
                    _end();
                    return;
                }

                state = kRVMWD177XWriteTrackStart;
                stepState = kRVMWD177XStepWaitIndex;
                control |= kRVMWD177XDRQ;
                wtrackmark = 0;
                //_do();
            }
            else if ((command & 0xf0) == 0xe0)
            { // Read Track

                state = kRVMWD177XReadTrackStart;
                stepState = kRVMWD177XStepWaitIndex;
            }

            return;
        }

        case kRVMWD177XReadSectorHeader:
        {

            if (header[0] != 0xfe)
            {
                state = kRVMWD177XReadHeader;
                _do();
                return;
            }

            //   printf("--------------------\n");
            //   printf(" Header track, sector and side: %d, %d, %d\n",header[1],header[3],header[2]);
            //   printf("Desired track, sector and side: %d, %d, %d\n",track,sector,side);
            //   printf("Disk index: %d\n",disk[diskS]->indx);
            //   for (int i = 0; i < 7; i++) {
            //     printf("Header[%d]: %02x\n",i,header[i]);
            //   }
            //   printf("--------------------\n");

            if (header[1] != track)
            {
                state = kRVMWD177XReadHeader;
                _do();
                return;
            }

            if (!fastmode || command & 0x20)
            {

                if (header[3] != sector)
                {
                    state = kRVMWD177XReadHeader;
                    _do();
                    return;
                }

                if ((command & 0x2) && (header[2] & ((command >> 3) & 0x1)))
                {
                    state = kRVMWD177XReadHeader;
                    _do();
                    return;
                }
            }
            else
            {

                if ((command & 0x2) && (header[2] & ((command >> 3) & 0x1)))
                {
                    state = kRVMWD177XReadHeader;
                    _do();
                    return;
                }

                header[3] = sector;
                disks[selectedDiskIndex]->index = SECTOR_DATA_POSITION[sector - 1] + 39; // 5;
            }

            // if(crc) {

            //   printf("ReadSectorHeader CRC %08X\n",crc);

            //   status|=kRVMWD177XStatusCRC;
            //   state=kRVMWD177XReadHeader;
            //   _do();
            //   return;

            // } else {

            status &= ~kRVMWD177XStatusCRC;

            counter = 0x100; // Esto, en Betadisk, siempre será el tamaño del sector

            // _waitMark();
            // printf("Waitmark ReadSectorHeader\n");
            stepState = kRVMWD177XStepWaitingMark;
            marka = mark;
            //

            if (command & 0x20)
            {

                // printf("Writing Command: %02x Track:%d Sector:%d Size:%llu\n",command,track,sector,c);

                state = kRVMWD177XWriteDataFlag;
                control |= kRVMWD177XDRQ;
            }
            else
            {

                state = kRVMWD177XReadDataFlag;
            }

            return;

            // }
        }

        case kRVMWD177XReadDataFlag:
        {
            stepState = kRVMWD177XStepReadByte;
            state = kRVMWD177XReadDataFlag2;
            return;
        }

        case kRVMWD177XWriteDataFlag:
        {
            stepState = kRVMWD177XStepWriteByte;
            state = kRVMWD177XWriteData;
            control |= kRVMWD177XWriting;
            a = (command & 0x1) ? 0xf8 : 0xfb;
            // crc=crc(crc,a);
            return;
        }

        case kRVMWD177XWriteData:
        {

            // Verificar underrun ANTES de procesar el byte
            if (control & kRVMWD177XDRQ)
            {
                // printf("Lost data in write - aborting command\n");

                // Flush written data to disk
                if (disks[selectedDiskIndex]->sectorBufferIndex < 0xff)
                {
                    disks[selectedDiskIndex]->dataIndex -= 0x100;                                                             // fseek(disk[diskS]->Diskfile,-0x100,SEEK_CUR);
                    writeDiskData(disks[selectedDiskIndex], disks[selectedDiskIndex]->sectorBuffer, disks[selectedDiskIndex]->sectorBufferIndex); // fwrite(disk[diskS]->cursectbuf,1,disk[diskS]->cursectbufpos,disk[diskS]->Diskfile);
                }

                status |= kRVMWD177XStatusLostData;
                control &= ~kRVMWD177XWriting;
                _end();
                return;
            }

            led = 2;

            a = data;
            // crc=crc(crc,a);
            // printf("Write %d data: %02x CRC: %04x\n",c,a,crc);
            data = 0;

            if (--counter)
            {
                control |= kRVMWD177XDRQ;
            }
            else
            {
                state = kRVMWD177XWriteCRC1;
                //_do();
            }
            return;
        }

        case kRVMWD177XWriteCRC1:
        {
            // a=crc>>8;
            // printf("Write CRC byte: %02x CRC: %04x\n",a,crc);
            state = kRVMWD177XWriteCRC2;
            return;
        }

        case kRVMWD177XWriteCRC2:
        {
            // a=crc & 0xff;
            // printf("Write CRC byte: %02x CRC: %04x\n",a,crc);
            state = kRVMWD177XWriteLast;

            return;
        }

        case kRVMWD177XWriteLast:
        {
            state = kRVMWD177XWriteEnd;
            stepState = kRVMWD177XStepLastWriteByte;
            return;
        }

        case kRVMWD177XWriteEnd:
        {

            control &= ~kRVMWD177XWriting;

            // Write buffer to diskfile
            // int saveptr = ftell(disk[diskS]->Diskfile);
            // int seekptr = (track << (11 + disk[diskS]->sides)) + (side ? 4096 : 0) + ((sector - 1) << 8);
            // fseek(disk[diskS]->Diskfile,seekptr,SEEK_SET);
            // fwrite(disk[diskS]->cursectbuf,1,0x100, disk[diskS]->Diskfile);
            // fseek(disk[diskS]->Diskfile,saveptr,SEEK_SET);

            // printf("Track:%d, Side:%d, Sector: %d\n",track,side,sector);
            // for (int i=0; i< 0x100; i+= 0x10) {
            //   printf("Pos %04x: ",i);
            //   for (int n=0; n< 0x10; n++) {
            //     printf("%02x ",disk[diskS]->cursectbuf[i + n]);
            //   }
            //   printf("\n");
            // }
            // printf("==================================\n");

            if (command & 0x10)
            { // Write sector: Multiple record flag on
                sector++;
                state = kRVMWD177XTypeIICommand;
                _do();
            }
            else
            { // Write sector: Multiple record flag off
                _end();
            }
            return;
        }

        case kRVMWD177XReadDataFlag2:
        {

            if (a == 0xf8)
            {
                status |= kRVMWD177XStatusRecordType;
            }
            else if (a == 0xfb)
            {
                status &= ~kRVMWD177XStatusRecordType;
            }
            else
            {
                state = kRVMWD177XTypeIICommand;
                _do();
                stepState = kRVMWD177XNone;
                return;
            }
            state = kRVMWD177XReadData;
            return;
        }

        case kRVMWD177XReadData:
        {

            led = 1;

            data = a;

            // printf("Read %d byte: %02x CRC: %04x\n",c,a,crc);
            if (control & kRVMWD177XDRQ)
            {
                // printf("Lost data in read\n");
                status |= kRVMWD177XStatusLostData;
            }

            control |= kRVMWD177XDRQ;
            if (!--counter)
            {
                state = kRVMWD177XReadCRC;
                counter = 2; // 2 bytes CRC
                return;
            }
            return;
        }

        case kRVMWD177XReadCRC:
        {
            // printf("Read CRC byte: %02x CRC: %04x\n",a,crc);
            if (!--counter)
            { // CRC readed

                // if(crc) {
                //   status|=kRVMWD177XStatusCRC;
                // } else {
                status &= ~kRVMWD177XStatusCRC;
                // }

                if (command & 0x10)
                { // Read sector: Multiple record flag on

                    sector++; // Next sector

                    if (!fastmode || sector > 16)
                    {

                        // printf("Fast mode Next sector: ");
                        // if (sector > 16)
                        //   printf("Sector >  16: %d\n",sector);
                        // else
                        //   printf("Sector <= 16: %d\n",sector);

                        // printf("Read Sector, multiple: sector %d\n",sector);

                        state = kRVMWD177XTypeIICommand;
                        _do();
                    }
                    else
                    {

                        header[3] = sector;
                        disks[selectedDiskIndex]->index = SECTOR_DATA_POSITION[sector - 1] + 39; // + 5;

                        status &= ~kRVMWD177XStatusCRC;
                        counter = 0x100; // Esto, en Betadisk, siempre será el tamaño del sector

                        // _waitMark();
                        stepState = kRVMWD177XStepWaitingMark;
                        marka = mark;
                        //

                        state = kRVMWD177XReadDataFlag;
                    }
                }
                else
                { // Read sector: Multiple record flag off

                    _end();
                }
            }

            return;
        }

        case kRVMWD177XWriteTrackStart:
        {
            if (control & kRVMWD177XDRQ)
            {
                status |= kRVMWD177XStatusLostData;
                _end();
                return;
            }

            control |= kRVMWD177XWriting;
            state = kRVMWD177XWriteTrack;

            disks[selectedDiskIndex]->index = 0xffffffff;
            disks[selectedDiskIndex]->sectorBufferIndex = 0xff;
            disks[selectedDiskIndex]->indexDelay = 0;

            _do();
            return;
        }

        case kRVMWD177XWriteTrack:
        {

            if (!retry)
            {
                _end();
                return;
            }

            // Verificar underrun ANTES de procesar
            if (control & kRVMWD177XDRQ)
            {
                // printf("Lost data in write track - aborting command\n");

                // Flush written data to disk
                if (disks[selectedDiskIndex]->sectorBufferIndex < 0xff)
                {
                    disks[selectedDiskIndex]->dataIndex -= 0x100;                                                             // fseek(disk[diskS]->Diskfile,-0x100,SEEK_CUR);
                    writeDiskData(disks[selectedDiskIndex], disks[selectedDiskIndex]->sectorBuffer, disks[selectedDiskIndex]->sectorBufferIndex); // fwrite(disk[diskS]->cursectbuf,1,disk[diskS]->cursectbufpos,disk[diskS]->Diskfile);
                }

                status |= kRVMWD177XStatusLostData;
                control &= ~kRVMWD177XWriting;
                _end();
                return;
            }

            led = 2;

            // // e=8;
            // if(control & kRVMWD177XDRQ) {

            //   a = 0;
            //   // crc=crc(crc,a);
            //   stepState=kRVMWD177XStepWriteByte;
            //   status|=kRVMWD177XStatusLostData;
            //   control|=kRVMWD177XDRQ;

            //   // printf("kRVMWD177XWriteTrack kRVMWD177XStatusLostData -> !! ->");

            // } else {

            // if (disk[diskS]->indx >0 && disk[diskS]->indx < 32) {
            //   printf("Format byte: %02x, ",a);
            //   printf("Disk index %d\n",disk[diskS]->indx);
            //   // printf("Disk index delay: %d\n",disk[diskS]->indexDelay);
            //   delay(125);
            // }

            switch (data)
            {

            case 0xf5:
            {
                // aa=sectorMark;
                // // e=16;
                // stepState=kRVMWD177XStepWriteRaw;
                // control|=kRVMWD177XDRQ;
                // // crc=0xcdb4;
                wtrackmark++;

                a = sectorMark;
                stepState = kRVMWD177XStepWriteByte;
                control |= kRVMWD177XDRQ;
                // printf("kRVMWD177XWriteTrack -> Sector Mark!! ->");
                break;
                // return;
            }

                // case 0xf6: {

                //   wtrackmark=0;

                //   // aa=indexMark;
                //   // // e=16;
                //   // stepState=kRVMWD177XStepWriteRaw;
                //   // control|=kRVMWD177XDRQ;

                //   a=indexMark;
                //   stepState=kRVMWD177XStepWriteByte;
                //   control|=kRVMWD177XDRQ;

                //   // printf("kRVMWD177XWriteTrack -> Index Mark!! ->");
                //   break;
                //   // return;
                // }

            case 0xf7:
            {

                wtrackmark = 0;

                // a=crc>>8;
                a = 0;
                stepState = kRVMWD177XStepWriteByte;
                state = kRVMWD177XWriteTrackCRC;
                // printf("kRVMWD177XWriteTrack -> CRC!! ->");
                break;
                // return;
            }

            default:
            {

                if (wtrackmark == 3 && data == 0xfe)
                {
                    // printf("Write Sector Header, track %d, side %d\n",track,side);
                    wtrackmark = 0b100000000;
                }
                else if (wtrackmark == 3 && data == 0xfb)
                {
                    // printf("Write Sector Data at sector %d\n",wtracksector);
                    // wtrackmark=0b1000000000;
                    disks[selectedDiskIndex]->index = SECTOR_DATA_POSITION[wtracksector - 1] + 41;
                }
                else if (wtrackmark & 0b100000000)
                {
                    wtrackmark++;
                    // printf("   Write Sector Header, byte: %02x\n",data);
                    if (wtrackmark == 0b100000001)
                    {
                        // printf("Write track to track0 side1 sector header!\n");
                        if (track == 0 && side == 1)
                            disks[selectedDiskIndex]->track0side1data = data;
                    }
                    else
                        // if (wtrackmark == 0b100000011) {
                        //   // side = data;
                        // }
                        if (wtrackmark == 0b100000011)
                        {
                            wtracksector = data;
                            wtrackmark = 0;
                        }
                    /*} else if (wtrackmark & 0b1000000000) {
                    // int seekptr = (track << (11 + disk[diskS]->sides)) + (side ? 4096 : 0) + ((wtracksector - 1) << 8);
                    // fseek(disk[diskS]->Diskfile,seekptr,SEEK_SET);
                    wtrackmark=0b10000000000;
                    } else if (wtrackmark & 0b10000000000) {
                    // int seekptr = (track << (11 + disk[diskS]->sides)) + (side ? 4096 : 0) + ((wtracksector - 1) << 8);
                    // fseek(disk[diskS]->Diskfile,seekptr,SEEK_SET);
                    // wtrackmark=0;
                    // fwrite(&data,1,1, disk[diskS]->Diskfile);
                    wtrackmark++;
                    if (wtrackmark & 0b100000000) wtrackmark = 0;*/
                }
                else
                {
                    wtrackmark = 0;
                }

                a = data;
                // crc=crc(crc,a);
                // printf("Format byte: %02x\n",a);
                stepState = kRVMWD177XStepWriteByte;
                control |= kRVMWD177XDRQ;
                break;
                // return;
            }
            }

            // }

            // printf("Format byte: %02x\n",a);

            return;
        }

        case kRVMWD177XWriteTrackCRC:
        {
            // a=crc & 0xff;
            a = 0x0;
            stepState = kRVMWD177XStepWriteByte;
            state = kRVMWD177XWriteTrack;
            control |= kRVMWD177XDRQ;

            // disk[diskS]->indx -= 4;
            // disk[diskS]->cursectbufpos = 0xffff;

            // printf("kRVMWD177XWriteTrack -> CRC!! ->");
            // printf("Format byte: %02x\n",a);
            return;
        }

        case kRVMWD177XReadTrackStart:
        {
            stepState = kRVMWD177XStepReadByte;
            state = kRVMWD177XReadTrackData;
            retry = 1;
            // printf("ReadTrackStart!\n");
            return;
        }

        case kRVMWD177XReadTrackData:
        {

            // printf("ReadTrackData!\n");

            if (!retry)
            {
                _end();
                return;
            }

            led = 1;

            if (control & kRVMWD177XDRQ)
                status |= kRVMWD177XStatusLostData;

            control |= kRVMWD177XDRQ;
            data = a;
            return;
        }
        }
    }

    void rvmWD1793Write(uint16_t addr, uint8_t value)
    {
        switch (addr & 0x00ff)
        {

        case 0x1f: // Command

            if ((value & 0xf0) == 0xd0)
            {

                // Force interrupt
                if (status & kRVMWD177XStatusBusy)
                {

                    status &= ~kRVMWD177XStatusBusy;
                    state = kRVMWD177XNone;
                    stepState = kRVMWD177XStepIdle;
                    control &= ~(kRVMWD177XWriting | kRVMWD177XDRQ);
                    retry = 15;
                    led = 0;
                }
                else
                {

                    status = kRVMWD177XStatusSetIndex | kRVMWD177XStatusSetTrack0 | kRVMWD177XStatusSetWP;
                }

                if ((value & 0xf) == 0x0)
                {

                    control &= ~(kRVMWD177XINTRQ | kRVMWD177XFINTRQ);
                }
                else
                {

                    if (value & 0x8)
                    {

                        // Inmediate interrupt
                        control |= kRVMWD177XFINTRQ;
                    }
                    else
                    {

                        control = (control & 0xffff) | ((value & 0xf) << 16); // Set conditions
                    }
                }

                return;
            }

            if (!(status & kRVMWD177XStatusBusy))
            {

                control &= ~(kRVMWD177XINTRQ | kRVMWD177XFINTRQ);

                command = value;

                // printf("COMMAND: %02x TRACK: %02x SECTOR %02x\n",command,track,sector);

                if (disks[selectedDiskIndex] && (control & (kRVMWD177XPower0 << selectedDiskIndex)))
                {

                    // Issue command
                    if (command & kRVMWD177XTypeI)
                    {

                        // printf("TYPE II,III or IV COMMAND: %02x TRACK: %02x SECTOR %02x SIDE %02x\n",command,track,sector, side);

                        // Type II, III, IV kRVMWD177XTypeIISetHead
                        if ((command & 0xc0) == 0x80 || (command & 0xfb) == 0xc0 || (command & 0xfb) == 0xf0 || (command & 0xfb) == 0xe0)
                        {

                            // TYPE
                            state = kRVMWD177XTypeIISetHead;
                            status = kRVMWD177XStatusBusy;

                            if (command & kRVMWD177XVerifyBit)
                            {

                                stepState = kRVMWD177XStepWaiting;

                                // if (fastmode)
                                //   c = 1;
                                // else
                                counter = 937; // 7500 (Value for 1 bit per diskstep) / 8
                                             // printf("Waiting 30ms\n");
                            }
                            else
                            {

                                _do();
                            }
                        }
                        else
                        {

                            control |= kRVMWD177XINTRQ;
                        }
                    }
                    else
                    {

                        // printf("TYPE I COMMAND: %02x TRACK: %02x SECTOR %02x SIDE %02x\n",command,track,sector, side);

                        // Type I
                        status = kRVMWD177XStatusSetIndex | kRVMWD177XStatusSetTrack0 | kRVMWD177XStatusSetWP | kRVMWD177XStatusBusy;
                        state = kRVMWD177XTypeI0;

                        _do();
                    }
                }
                else
                {

                    status = kRVMWD177XStatusNotReady;
                    control |= kRVMWD177XINTRQ;
                }

            } // else printf("BUSY!!!\n");

            break;

        case 0x3f: // Track
                // if(!(status & kRVMWD177XStatusBusy)) {
            track = value;
            //}
            break;
        case 0x5f: // Sector
                // if(!(status & kRVMWD177XStatusBusy)) {
            sector = value;
            //}
            break;
        case 0x7f: // Data
            data = value;
            control &= ~kRVMWD177XDRQ;
            break;
        case 0x00ff:
            // FDDStep(true);

            // Change active disk unit
            if (selectedDiskIndex != (value & 0x3))
            {
                selectedDiskIndex = value & 0x3;
                if (disks[selectedDiskIndex] != NULL && side && disks[selectedDiskIndex]->sideCount == 1)
                    side = 0;
                sclConverted = false;
            }

            if (!(value & 0x4))
            {
                rvmWD1793Reset();
            }

            if (value & 0x8)
                control |= kRVMWD177XTest;
            else
                control &= ~kRVMWD177XTest;

            if (value & 0x10)
                side = 0;
            else
            {
                if (disks[selectedDiskIndex] != NULL)
                    side = disks[selectedDiskIndex]->sideCount == 1 ? 0 : 1;
                else
                    side = 1;
            }

            if (value & 0x40)
                control |= kRVMWD177XDDEN;
            else
                control &= ~kRVMWD177XDDEN;
                
            break;
        }
    }

    uint8_t rvmWD1793Read(uint16_t addr)
    {
        uint8_t r;

        switch (addr & 0x00ff)
        {
        case 0x1f: // Status
            control &= ~kRVMWD177XINTRQ;

            r = status & 0xff;

            if (disks[selectedDiskIndex])
            {
                if (status & kRVMWD177XStatusSetWP)
                {
                    if (disks[selectedDiskIndex]->writeProtect)
                    {
                        r |= kRVMWD177XStatusProtected;
                    }
                    else
                    {
                        r &= ~kRVMWD177XStatusProtected;
                    }
                }

                if ((status & kRVMWD177XStatusSetTrack0) && (disks[selectedDiskIndex]->s & kRVMwdDiskOutTrack0))
                {
                    r |= kRVMWD177XStatusTrack0;
                }

                if ((status & kRVMWD177XStatusSetIndex) && (disks[selectedDiskIndex]->s & kRVMwdDiskOutIndex))
                {
                    r |= kRVMWD177XStatusIndex;
                }
                else
                {
                    if (control & kRVMWD177XDRQ)
                        r |= kRVMWD177XStatusDataRequest;
                }

                if (status & kRVMWD177XStatusSetHead)
                {
                    r |= kRVMWD177XStatusHeadLoaded;
                }
            }
            else
            {
                r |= kRVMWD177XStatusNotReady;
            }

            // printf("Read status: %02x\n",r);
            return r;
        case 0x3f: // Track
            return track;
        case 0x5f: // Sector
            return sector;
        case 0x7f: // Data

            // if(!(control&kRVMWD177XDRQ)) {
            //   printf("Read data register overrunning\n");
            // }

            control &= ~kRVMWD177XDRQ;

            // printf("read data: %02x\n", data);
            return data;
        case 0x00ff:
        {
            //FDDStep(true);
            uint8_t v = 0;
            if (control & kRVMWD177XDRQ) v |= 0x40;
            if (control & (kRVMWD177XINTRQ | kRVMWD177XFINTRQ)) v |= 0x80;
            return v;
        }
        }

        return 0;
    }

    void rvmWD1793Step()
    {
        if ((main::currentTack % WD177XSTEPSTATES) != 0 || (control & (kRVMWD177XHLD | kRVMWD177XHLT) == 0))
        {
            return;
        }

        uint8_t dd = 0x0;
        uint8_t s = 0x0;

        // uint8_t s=0x0;
        // uint8_t dd=0x0;

        if (disks[selectedDiskIndex])
        { // If active disk exists ..

            // uint16_t w = 0;
            // if((control & kRVMWD177XWriting) && !disk[diskS]->writeprotect) {
            //   w |= kRVMwdDiskControlWrite | wb;
            // }
            // rvmwdDiskStep(wd, w);

            if ((control & kRVMWD177XWriting) && !disks[selectedDiskIndex]->writeProtect)
            {
                rvmwdDiskStep_write(wb);
                dd = 0;
            }
            else
            {
                rvmwdDiskStep_read();
                dd = disks[selectedDiskIndex]->a;
            }

            s = disks[selectedDiskIndex]->s;
        }

        uint8_t pd = s ^ previousDiskIndex;
        previousDiskIndex = s;

        // printf("stepState: %d\n",stepState);

        switch (stepState)
        {

        case kRVMWD177XStepIdle:
        {

            if (retry && (pd & kRVMwdDiskOutIndex) && (s & kRVMwdDiskOutIndex))
            {
                retry--;
                if (!retry)
                {
                    control &= ~(kRVMWD177XHLD | kRVMWD177XHLT);
                    return;
                }
            }
            break;
        }

        case kRVMWD177XStepWaiting:
        {

            if (fastmode)
            {
                counter = 0;
                _do();
            }
            else if (!(--counter))
            {
                _do();
            }

            break;
        }

        case kRVMWD177XStepWaitingMark:
        {

            // _checkIndex(wd,pd,s);
            if (retry && (pd & kRVMwdDiskOutIndex) && (s & kRVMwdDiskOutIndex))
            {
                retry--;
                if (!retry)
                    _do();
            }
            // end _checkIndex

            if ((marka & 0xff) == dd)
            {
                marka >>= 8;
                if (!marka)
                {
                    // printf("Mark found for Track: %d, Sector: %d, Index: %d!\n",track, sector, disk[diskS]->indx);
                    _do();
                    if (control & kRVMWD177XWriting)
                        goto write;
                }
            }
            else
            {
                marka = mark;
            }

            break;
        }

        case kRVMWD177XStepReadByte:
        {

            // _checkIndex(wd,pd,s);
            if (retry && (pd & kRVMwdDiskOutIndex) && (s & kRVMwdDiskOutIndex))
            {
                retry--;
                if (!retry)
                    _do();
            }
            // end _checkIndex

            a = dd;
            _do();
            a = 0;

            break;
        }

        case kRVMWD177XStepWriteByte:
        {

        write:

            // _checkIndex(wd,pd,s);
            if (retry && (pd & kRVMwdDiskOutIndex) && (s & kRVMwdDiskOutIndex))
            {
                retry--;
                if (!retry)
                    _do();
            }
            // end _checkIndex

            wb = a;
            _do();

            break;
        }

        case kRVMWD177XStepWriteRaw:
        {

            // _checkIndex(wd,pd,s);
            if (retry && (pd & kRVMwdDiskOutIndex) && (s & kRVMwdDiskOutIndex))
            {
                retry--;
                if (!retry)
                    _do();
            }
            // end _checkIndex

            wb = a;
            _do();

            break;
        }

        case kRVMWD177XStepLastWriteByte:
            _do();
            break;

        case kRVMWD177XStepWaitIndex:
        {
            if ((pd & kRVMwdDiskOutIndex) && (s & kRVMwdDiskOutIndex))
            {
                retry = 1;
                _do();
            }
            break;
        }
        }
    }

    void rvmWD1793Reset()
    {
        state = kRVMWD177XNone;
        stepState = kRVMWD177XStepIdle;
        nextState = kRVMWD177XNone;

        counter = 0;
        control &= 0xf000;
        command = sector = data = dsr = 0x0;
        status = kRVMWD177XStatusSetIndex | kRVMWD177XStatusSetTrack0 | kRVMWD177XStatusSetWP;
        track = 0xff;
        wtrackmark = 0;
        headerI = 0;
        retry = 0;
        // crc=0xffff;
        crc = 0; // Disable CRC. Not needed for Betadisk emulation
        side = 0;
        selectedDiskIndex = 0;
        fastmode = false;//(Config::DiskCtrl == 2 || Config::DiskCtrl == 4);
        //!!!!!!!!!!!!
        sclConverted = false;
    }

    bool rvmWD1793InsertDisk(unsigned char UnitNum, std::string Filename)
    {
        // Close any open disk in this unit
        wdDiskEject(UnitNum);

        disks[UnitNum] = new rvmwdDisk(); //(rvmwdDisk*)heap_caps_calloc(1, sizeof(rvmwdDisk), MALLOC_CAP_8BIT);

        std::ifstream file(Filename, std::ios::in | std::ios::binary);
        if (!file.is_open())
        {
            return false;
        }

        file.seekg(0, std::ios::end);
        std::streampos fileSize = file.tellg();
        disks[UnitNum]->dataLength = (int)fileSize;
        disks[UnitNum]->data = new uint8_t[disks[UnitNum]->dataLength];

        file.seekg(0, std::ios::beg);
        file.read((char *)disks[UnitNum]->data, fileSize);
        file.close();

        uint8_t diskType;

        std::string magic = std::string((char *)(disks[UnitNum]->data), 8);

        if (magic == "SINCLAIR")
        {

            // SCL file
            printf("SCL disk loaded\n");
            disks[UnitNum]->scl = true;
            disks[UnitNum]->writeProtect = true; // SCL files are read only
            diskType = 0x16;
        }
        else
        {

            disks[UnitNum]->scl = false;
            disks[UnitNum]->writeProtect = false; // Maybe I should consider that TRD files are loaded read only by default and add an option to open them read/write
            disks[UnitNum]->sclDataOffset = 0;
            diskType = disks[UnitNum]->data[2048 + 227];
        }

        switch (diskType)
        {
        case 0x16:
            disks[UnitNum]->trackCount = 79;
            disks[UnitNum]->sideCount = 2;
            break;
        case 0x17:
            disks[UnitNum]->trackCount = 39;
            disks[UnitNum]->sideCount = 2;
            break;
        case 0x18:
            disks[UnitNum]->trackCount = 79;
            disks[UnitNum]->sideCount = 1;
            break;
        case 0x19:
            disks[UnitNum]->trackCount = 39;
            disks[UnitNum]->sideCount = 1;
            break;
        default:
            wdDiskEject(UnitNum);
            return false;
        }

        // Check if we have more tracks than on a standard disk
        if (!disks[UnitNum]->scl)
        {
            // Get file size
            if (disks[UnitNum]->dataLength > disks[UnitNum]->sideCount * disks[UnitNum]->trackCount * 16 * 256)
            {
                int i;
                for (int i = disks[UnitNum]->trackCount + 1; i < 83; i++)
                {
                    if (disks[UnitNum]->sideCount * i * 16 * 256 >= disks[UnitNum]->dataLength)
                    {
                        disks[UnitNum]->trackCount = i;
                        break;
                    }
                }
            }
        }

        disks[UnitNum]->dataIndex = 0; // rewind(disk[UnitNum]->Diskfile);

        disks[UnitNum]->track0side1data = 0;
        disks[UnitNum]->sectorBufferIndex = 0xff; // 0xffff;

        // Power on drive
        control |= kRVMWD177XPower0 << UnitNum;

        printf("Disk %d inserted! Disktype: %d\n", UnitNum, (int)diskType);

        // disk[UnitNum]->fname = Filename;

        return true;
    }

    void wdDiskEject(unsigned char UnitNum)
    {
        if (disks[UnitNum] != NULL)
        {

            printf("Ejecting disk\n");

            // if (disk[UnitNum]->Diskfile != NULL) {
            //     fclose(disk[UnitNum]->Diskfile);
            //     disk[UnitNum]->Diskfile = NULL;
            // }

            // free(disk[UnitNum]);
            delete[disks[UnitNum]->dataLength] disks[UnitNum]->data;
            disks[UnitNum] = NULL;

            if (selectedDiskIndex == UnitNum)
            {

                _end();

                side = 0;

                sclConverted = false;
            }

            // Power off drive
            control &= ~(kRVMWD177XPower0 << UnitNum);
        }
        else
            printf("No disk to eject\n");
    }
}
