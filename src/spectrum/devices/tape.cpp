#include <fstream>
#include <vector>
#include <format>

#include "tape.h"
#include "win_app.h"

namespace tape
{
    int dataLength;
    uint8_t* data;

    enum Format { TAP, TZX };
    Format format;

    struct Block
    {
        uint8_t type;
        int start;
        int length;
        std::string name;
        std::string description;
    };

    std::vector<Block> blocks;

    std::string getDataInfo(int index, int blockLength)
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

        return "";
    }

    static void tzxParseBlocks()
    {
        blocks.clear();

        int groupIndex;
        int groupLength;
        int groupCount = 0;
        std::string groupName;
        int index = 10;
        while (index < dataLength)
        {
            const uint8_t type = data[index++];
            switch (type)
            {

            // Standard Speed Data Block
            case 0x10:
            {
                const uint16_t blockLength = data[index + 2] | data[index + 3] << 8;
                blocks.emplace_back(type, index, blockLength, "Standard block", getDataInfo(index + 4, blockLength));
                index += 4;
                index += blockLength;
                break;
            }

            // Turbo Speed Data Block
            case 0x11:
            {
                const uint32_t blockLength = data[index + 15] | (data[index + 16] << 8) | data[index + 17] << 16;
                blocks.emplace_back(type, index, blockLength, "Turbo block", getDataInfo(index + 18, blockLength));
                index += 18;
                index += blockLength;
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
                groupLength +=blockLength;
                index += 10;
                index += blockLength;
                break;
            }

            // Pause
            case 0x20:
            {
                const uint16_t duration = data[index] | data[index + 1] << 8;
                blocks.emplace_back(type, index, 2, "Pause", std::format("{}s", duration / 1000));
                index += 2;
                break;
            }

            // Group start
            case 0x21:
            {
                groupIndex = index;
                groupLength = 0;
                const uint8_t blockLength = data[index++];
                groupName = std::string(data + index, data + index + blockLength);
                index += blockLength;
                break;
            }

            // Group end
            case 0x22:
            {
                blocks.emplace_back(0x21, groupIndex, groupLength, "Group", groupName.empty() ? std::format("block {}", ++groupCount) : groupName);
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
            
            // Text description
            case 0x30: 
            {
                const uint8_t blockLength = data[index++];
                blocks.emplace_back(type, index, blockLength, "Description", std::string(data + index, data + index + blockLength));
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
                index += dataLength;
                break;
            }

            default:
                win_app::info("tape block type not implemented");
                break;

            }
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
            format = TZX;
            tzxParseBlocks();
            return;
        }

        // int index = 0;
        // while (index < tapeDataLength)
        // {
        //     int blockLength = _tapeData[index++];
        //     blockLength |= (_tapeData[index++] << 8);

        //     index += blockLength;

        //     if (index == tapeDataLength)
        //     {
        //         _tapeformat = TapeFormat::tap;
        //         buildTapBlockList();
        //         return;
        //     }
        // }

        // // invalid format !!!
        // cleanUp();
    }

    void cleanUp()
    {
        // stop(false);

        if (dataLength)
        {
            delete[dataLength] data;
            data = nullptr;
        }

        // _tapeDataIndex = 0;
        // tapeBlockIndex = 0;
        // tapeDataLength = 0;
        // info.clear();
        // blockIndexesList.clear();
        // blocksList.clear();
    }
}