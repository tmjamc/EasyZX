#include <fstream>
#include <vector>
#include <format>
#include <ranges>

#include "tape.h"
#include "ula.h"
#include "win_app.h"
#include "settings.h"

namespace tape
{
    namespace
    {
        constexpr int PILOT_PULSES_HEADER_COUNT = 8063;
        constexpr int PILOT_PULSES_DATA_COUNT = 3223;
        constexpr int PILOT_PULSES_LENGTH = 2168;
        constexpr int SYNC_PULSE_1_LENGTH = 667;
        constexpr int SYNC_PULSE_2_LENGTH = 735;
        constexpr int PULSE_ZERO_LENGTH = 855;
        constexpr int PULSE_ONE_LENGTH = 1710;
        constexpr int BLOCK_PAUSE_CYCLES = 69888 * 50;

        int dataLength;
        uint8_t* data = nullptr;

        enum Format { TAP, TZX };
        Format fileFormat;

        struct Block
        {
            uint8_t type;
            int start;
            std::string description;
            int length;
        };

        std::vector<Block> blocks;

        int blockIndex = 0;

        struct Pulse
        {
            int length; // pulse length in z80 cycles
            int count;  // repeat this pulse "count" times. Each time a pulse is played whatever in this struct or the next one, the signal changes
                        // count = -1 means a pause of "length" cycles
        };

        std::vector<Pulse> pulses;
        int pulseIndex = 0;
        Pulse pulse{};

        std::string getDataInfo(int index, int blockLength, std::string defaultValue)
        {
            char fileName[11]{};

            if (blockLength == 19 && data[index] == 0)
            {
                int nameIndex = index + 2;
                int j = 0;
                for (int i = 0; i < 10; ++i)
                {
                    const char c = data[nameIndex + i];
                    if (c >= 0x20 && c <= 0x7f)
                    {
                        if (j == 0 && c == 0x20)
                        {
                            continue;
                        }

                        fileName[j++] = c;
                    }
                }

                for (int i = j - 1; i > 0; --i)
                {
                    if (fileName[i] == 0x20 || fileName[i] == 0x00)
                    {
                        fileName[i] = 0;
                    }
                    else
                    {
                        break;
                    }
                }

                switch (data[index + 1])
                {
                case 0:
                    return std::format("PROGRAM: {}", fileName);
                case 1:
                    return std::format("NUMERIC ARRAY: {}", fileName);
                case 2:
                    return std::format("CHARACTER ARRAY: {}", fileName);
                case 3:
                    return std::format("BYTES: {}", fileName);
                }
            }

            return defaultValue;
        }

        void tzxParseBlocks()
        {
            blocks.clear();

            int groupIndex = 0;
            int groupLength = 0;
            int groupCount = 0;
            bool inGroup = false;
            std::string groupName;
            int index = 10;
            bool latestBlockWasStandardHeader = false;
            while (index < dataLength)
            {
                const uint8_t type = data[index++];
                switch (type)
                {

                // Standard Speed Data Block
                case 0x10:
                {
                    const uint16_t blockLength = data[index + 2] | data[index + 3] << 8;
                    
                    if (latestBlockWasStandardHeader)
                    {                    
                        Block* latestBlock = &blocks[blocks.size() - 1];
                        latestBlock->length = blockLength;
                        latestBlockWasStandardHeader = false;
                    }
                    else
                    {
                        if (!inGroup)
                        {
                            blocks.emplace_back(type, index - 1, getDataInfo(index + 4, blockLength, "Standard Speed Data Block"), blockLength);
                        }
                        Block* currentBlock = &blocks[blocks.size() - 1];
                        if (currentBlock->length == 19 && !currentBlock->description.empty())
                        {
                            latestBlockWasStandardHeader = true;
                        }
                    }
                    index += 4;
                    index += blockLength;
                    groupLength += blockLength;
                    break;
                }

                // Turbo Speed Data Block
                case 0x11:
                {
                    const uint32_t blockLength = data[index + 15] | (data[index + 16] << 8) | data[index + 17] << 16;
                    if (!inGroup)
                    {
                        blocks.emplace_back(type, index - 1, getDataInfo(index + 18, blockLength, "Turbo Speed Data Block"), blockLength);
                    }
                    index += 18;
                    index += blockLength;
                    groupLength += blockLength;
                    break;
                }

                // Pure tone
                case 0x12:
                {
                    index += 4;
                    break;
                }

                // Pulse sequence
                case 0x13:
                {
                    uint8_t count = data[index++];
                    index += (2 * count);
                    break;
                }

                // Pure Data Block
                case 0x14:
                {
                    const uint32_t blockLength = data[index + 7] | (data[index + 8] << 8) | data[index + 9] << 16;
                    index += 10;
                    index += blockLength;
                    groupLength += blockLength;
                    break;
                }

                // Pause or stop the tape
                case 0x20:
                {
                    const uint16_t duration = data[index] | data[index + 1] << 8;
                    if (!inGroup)
                    {
                        if (duration == 0)
                        {
                            blocks.emplace_back(0x2a, index - 1, "Stop the tape", 0);
                        }
                        else
                        {
                            blocks.emplace_back(type, index - 1, std::format("Pause {}s", duration / 1000), 0);
                        }
                    }
                    index += 2;
                    break;
                }

                // Group start
                case 0x21:
                {
                    groupIndex = blocks.size();
                    groupLength = 0;
                    const uint8_t blockLength = data[index++];
                    groupName = std::string(data + index, data + index + blockLength);
                    blocks.emplace_back(type, index - 2, groupName.empty() ? std::format("group {}", ++groupCount) : groupName, 0);
                    index += blockLength;
                    inGroup = true;
                    break;
                }

                // Group end
                case 0x22:
                {
                    blocks[groupIndex].length = groupLength;
                    inGroup = false;
                    break;
                }

                // Loop start
                case 0x24:
                {
                    index += 2;
                    break;
                }

                // Loop end
                case 0x25:
                {
                    break;
                }

                // Stop the tape
                case 0x2a:
                {
                    blocks.emplace_back(type, index - 1, "Stop the tape", 0);
                    index += 4;
                    break;
                }
                
                // Text description
                case 0x30: 
                {
                    const uint8_t blockLength = data[index++];
                    if (!inGroup)
                    {
                        blocks.emplace_back(type, index - 1, std::string(data + index, data + index + blockLength), 0);
                    }
                    index += blockLength;
                    break;
                }

                // Archive info
                case 0x32:
                {
                    const uint16_t blockLength = data[index] | data[index + 1] << 8;
                    index += 2;
                    index += blockLength;
                    break;
                }
                
                // Hardware type
                case 0x33:
                {
                    const uint16_t blockLength = data[index++] * 3;
                    index += blockLength;
                    break;
                }

                default:
                    win_app::info("tape block type not implemented");
                    return;

                }
            }
        }

        void tapParseBlocks()
        {
            blocks.clear();

            int index = 0;
            while (index < dataLength)
            {
                const uint16_t blockLength = data[index] | data[index + 1] << 8;
                blocks.emplace_back(0x10, index, getDataInfo(index + 2, blockLength, "Standard block"), blockLength);
                index += 2;
                index += blockLength;
            }
        }

        void addPausePulses(uint64_t pauseLength)
        {
            pulses.emplace_back((int)((pauseLength * 69888) / 20), 1);
        }

        void setDataPulses(uint16_t pulseZeroLength, uint16_t pulseOneLength, uint8_t lastByteBits, uint32_t pauseLength, uint32_t dataStartIndex, uint32_t dataLength)
        {
            uint32_t remainingBytes = dataLength;
            Pulse tapePulse = {0, 0};
            int bit = 7;

            while (true)
            {
                const int tapePulseLength = data[dataStartIndex] & (1 << bit) ? pulseOneLength : pulseZeroLength;
                if (tapePulse.length == tapePulseLength)
                {
                    tapePulse.count += 2;
                }
                else
                {
                    if (tapePulse.length)
                    {
                        pulses.push_back(tapePulse);
                    }
                    tapePulse.length = tapePulseLength;
                    tapePulse.count = 2;
                }

                if (remainingBytes == 1 && (8 - bit) == lastByteBits)
                {
                    ++dataStartIndex;
                    break;
                }

                if (--bit < 0)
                {
                    bit = 7;
                    --remainingBytes;
                    ++dataStartIndex;
                }
            }

            pulses.push_back(tapePulse);

            if (pauseLength)
            {
                addPausePulses(pauseLength);
            }
        }
        
        void setPilotAndDataPulses(uint16_t pilotPulseLength,
                                        uint16_t syncPulse1Length,
                                        uint16_t syncPulse2Length,
                                        uint16_t pulseZeroLength,
                                        uint16_t pulseOneLength,
                                        uint16_t pilotPulseCount,
                                        uint8_t lastByteBits,
                                        uint32_t pauseLength,
                                        uint32_t dataStartIndex,
                                        uint32_t dataLength)
        {
            pulses.emplace_back(pilotPulseLength, pilotPulseCount);
            pulses.emplace_back(syncPulse1Length, 1);
            pulses.emplace_back(syncPulse2Length, 1);

            setDataPulses(pulseZeroLength, pulseOneLength, lastByteBits, pauseLength, dataStartIndex, dataLength);
        }

        void setTzxPulses(int startDataIndex, int endDataIndex)
        {
            int tzxLoopCount = 0;
            int tzxLoopStartDataIndex = 0;

            while (startDataIndex < endDataIndex)
            {
                switch (data[startDataIndex++])
                {

                // Standard Speed Data Block
                case 0x10:
                {
                    const uint16_t pauseLength = data[startDataIndex] | data[startDataIndex + 1] << 8;
                    const uint16_t blockLength = data[startDataIndex + 2] | data[startDataIndex + 3] << 8;

                    startDataIndex += 4;

                    setPilotAndDataPulses(PILOT_PULSES_LENGTH,
                                        SYNC_PULSE_1_LENGTH,
                                        SYNC_PULSE_2_LENGTH,
                                        PULSE_ZERO_LENGTH,
                                        PULSE_ONE_LENGTH,
                                        data[startDataIndex] < 0x80 ? PILOT_PULSES_HEADER_COUNT : PILOT_PULSES_DATA_COUNT,
                                        8,
                                        pauseLength,
                                        startDataIndex,
                                        blockLength);

                    startDataIndex += blockLength;

                    break;
                }

                // Turbo Speed Data Block
                case 0x11: 
                {
                    const uint16_t pilotPulseLength = data[startDataIndex] | data[startDataIndex + 1] << 8;
                    const uint16_t syncPulse1Length = data[startDataIndex + 2] | data[startDataIndex + 3] << 8;
                    const uint16_t syncPulse2Length = data[startDataIndex + 4] | data[startDataIndex + 5] << 8;
                    const uint16_t pulseZeroLength = data[startDataIndex + 6] | data[startDataIndex + 7] << 8;
                    const uint16_t pulseOneLength = data[startDataIndex + 8] | data[startDataIndex + 9] << 8;
                    const uint16_t pilotPulseCount = data[startDataIndex + 10] | data[startDataIndex + 11] << 8;
                    const uint8_t lastByteBits = data[startDataIndex + 12];
                    const uint16_t pauseLength = data[startDataIndex + 13] | data[startDataIndex + 14] << 8;
                    const uint32_t blockLength = data[startDataIndex + 15] | (data[startDataIndex + 16] << 8) | data[startDataIndex + 17] << 16;

                    startDataIndex += 18;

                    setPilotAndDataPulses(pilotPulseLength,
                        syncPulse1Length,
                        syncPulse2Length,
                        pulseZeroLength,
                        pulseOneLength,
                        pilotPulseCount,
                        lastByteBits,
                        pauseLength,
                        startDataIndex,
                        blockLength);

                    startDataIndex += blockLength;

                    break;
                }

                // Pure tone
                case 0x12:
                {
                    const uint16_t toneLength = data[startDataIndex] | data[startDataIndex + 1] << 8;
                    const uint16_t toneCount = data[startDataIndex + 2] | data[startDataIndex + 3] << 8;
                    startDataIndex += 4;

                    pulses.emplace_back(toneLength, toneCount);

                    break;
                }

                // Pulse sequence
                case 0x13:
                {
                    uint8_t count = data[startDataIndex++];

                    while (count)
                    {
                        const uint16_t pulseLength = data[startDataIndex] | data[startDataIndex + 1] << 8;
                        pulses.emplace_back(pulseLength, 1);
                        startDataIndex += 2;
                        count--;
                    }

                    break;
                }

                // Pure Data Block
                case 0x14:
                {
                    const uint16_t pulseZeroLength = data[startDataIndex] | data[startDataIndex + 1] << 8;
                    const uint16_t pulseOneLength = data[startDataIndex + 2] | data[startDataIndex + 3] << 8;
                    const uint8_t lastByteBits = data[startDataIndex + 4];
                    const uint16_t pauseLength = data[startDataIndex + 5] | data[startDataIndex + 6] << 8;
                    const uint32_t blockLength = data[startDataIndex + 7] | (data[startDataIndex + 8] << 8) | data[startDataIndex + 9] << 16;
                    startDataIndex += 10;

                    setDataPulses(pulseZeroLength, pulseOneLength, lastByteBits, pauseLength, startDataIndex, blockLength);

                    startDataIndex += blockLength;

                    break;
                }

                // Pause
                case 0x20:
                {
                    const uint16_t pauseLength = data[startDataIndex] | data[startDataIndex + 1] << 8;
                    startDataIndex += 2;
                    addPausePulses(pauseLength);
                    break;
                }

                // Group start
                case 0x21:
                {
                    const uint8_t textLength = data[startDataIndex++];
                    startDataIndex += textLength;
                    break;
                }

                // Group end
                case 0x22:
                {
                    break;
                }

                // Loop start
                case 0x24:
                {
                    tzxLoopCount = data[startDataIndex] | data[startDataIndex + 1] << 8;
                    startDataIndex += 2;
                    tzxLoopStartDataIndex = startDataIndex;
                    break;
                }

                // Loop end
                case 0x25:
                {
                    if (tzxLoopCount--)
                    {
                        startDataIndex = tzxLoopStartDataIndex;
                    }
                    break;
                }

                default:
                    win_app::info("not implemented");
                    break;
                
                }
            }
        }

        bool setBlockPulses()
        {
            bool result = false;

            pulses.clear();
            pulseSignal = false;

            if (blockIndex == blocks.size())
            {
                blockIndex = 0;
                return false;
            }

            if (blocks[blockIndex].type == 0x2a)
            {
                win_app::info("Stop");
                ++blockIndex %= blocks.size();
                playing = false;
                return false;
            }

            while (blockIndex < blocks.size() && blocks[blockIndex].type == 0x30)
            {
                ++blockIndex;
            }

            int dataIndexStart = blocks[blockIndex].start;
            int dataIndexEnd = (blockIndex == blocks.size() - 1) ? dataLength : blocks[blockIndex + 1].start;
            
            win_app::info(std::format("{} {} {}", blockIndex, dataIndexStart, dataIndexEnd).c_str());

            switch (fileFormat)
            {
            case TZX:
                setTzxPulses(dataIndexStart, dataIndexEnd);
                break;
            case TAP:
                //setTapPulses(dataIndexStart, dataIndexEnd);
                break;
            default:
                return false;
            }

            pulseIndex = 0;
            pulse = pulses[0];

            return true;
        }

        void printBlocks()
        {
            
            int max = 0;
            for (auto block : blocks)
            {
                if (block.description.length() > max)
                {
                    max = block.description.length();
                }
            }

            const std::string fmt = std::format("{{:2d}} 0x{{:02x}} {{:5d}} {{:<{}}} {{:5d}}", max);
            
            win_app::info("");

            int index = 0;
            for (auto block : blocks)
            {
                win_app::info(std::vformat(fmt, std::make_format_args(index, block.type, block.start, block.description, block.length)).c_str());
                ++index;
            }

            win_app::info("");
        }
    }

    bool playing = false;
    bool pulseSignal = false;
    DcAdjustmentFilter filter;
    int volume;

    void reset()
    {
        filter.setBufferPercentLength(settings::current.audioTapeDcAdjustBufferLength);
        volume = (MAX_AMPLITUDE * settings::current.audioTapeVolume) / 200;

        playing = false;
        blockIndex = 0;
        pulses.clear();
        pulseIndex = 0;
        pulseSignal = false;
    }

    void cleanUp()
    {
        if (data != nullptr)
        {
            delete[] data;
            data = nullptr;
        }
    }
    
    void load(const char *fileName)
    {
        std::ifstream file(fileName, std::ios::in | std::ios::binary);
        if (!file.is_open())
        {
            return;
        }

        cleanUp();

        file.seekg(0, std::ios::end);
        std::streampos fileSize = file.tellg();
        dataLength = (int)fileSize;
        data = new uint8_t[dataLength];

        file.seekg(0, std::ios::beg);
        file.read((char*)data, fileSize);
        file.close();

        std::string signature((char*)data, dataLength < 7 ? dataLength : 7);

        if (signature == "ZXTape!")
        {
            fileFormat = TZX;
            tzxParseBlocks();
            printBlocks();

            // blockIndex = 5;

            return;
        }

        int index = 0;
        while (index < dataLength)
        {
            int blockLength = data[index++];
            blockLength |= (data[index++] << 8);

            index += blockLength;

            if (index == dataLength)
            {
                fileFormat = TAP;
                tapParseBlocks();
                printBlocks();
                return;
            }
        }

        // invalid format !!!
        cleanUp();
    }

    void play()
    {
        win_app::info("Play");

        if (playing || !setBlockPulses())
        {
            return;
        }

        playing = true;
    }

    void tact()
    {
        filter.add(pulseSignal ? volume : -volume);

        if (!playing)
        {
            return;
        }

        if (--pulse.length <= 0)
        {
            pulseSignal = !pulseSignal;
            // ++pulseIndex;

            if (--pulse.count > 0)
            {
                pulse.length = pulses[pulseIndex].length;
            }
            else
            {
                if (++pulseIndex >= pulses.size())
                {
                    ++blockIndex;

                    if (blockIndex == blocks.size())
                    {
                        playing = false;
                        //stop(true);
                        blockIndex = 0;
                    }
                    else
                    {
                        if (true /*!_zx->app->settings->tapeAutoStartStop || _zx->ula->latestPortReadWasTapeLoader()*/)
                        {
                            if (!setBlockPulses())
                            {
                                playing = false;
                                // stop(false);
                            }
                        }
                        // else
                        // {
                        //     playing = false;
                        //     // stop(false);
                        // }
                    }
                }
                else
                {
                    pulse = pulses[pulseIndex];
                }
            }
        }
    }
}