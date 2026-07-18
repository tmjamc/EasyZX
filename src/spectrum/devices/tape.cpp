#include <span>
#include <fstream>

#include "tape.h"
#include "z80.h"
#include "memory.h"

namespace tape
{
    int tapeDataLength = 0;
    int tapeDataIndex = 0;
    uint8_t* tapeData = nullptr;

    static std::span<uint8_t> getRawBlockData()
    {
        if (tapeDataLength == 0)
        {
            return std::span<uint8_t>();
        }

        std::span<uint8_t> result(tapeData, tapeDataLength);

        int blockLength = tapeData[tapeDataIndex++];
        blockLength |= (tapeData[tapeDataIndex++] << 8);
        result = result.subspan(tapeDataIndex, blockLength);
        tapeDataIndex += blockLength;

        if (tapeDataIndex >= tapeDataLength)
        {
            tapeDataIndex = 0;
        }

        return result;
    }

    void init()
    {
        // std::ifstream file("C:\\Users\\jam\\source\\repos\\RawZX\\tapes\\tests\\contention.tap", std::ios::in | std::ios::binary);
        // std::ifstream file("C:\\Users\\jam\\source\\repos\\RawZX\\tapes\\tests\\iocontention.tap", std::ios::in | std::ios::binary);
        // std::ifstream file("C:\\Users\\jam\\source\\repos\\RawZX\\tapes\\tests\\IR_Contention.tap", std::ios::in | std::ios::binary);
        // std::ifstream file("C:\\Users\\jam\\source\\repos\\RawZX\\tapes\\tests\\screen_timing_early.tap", std::ios::in | std::ios::binary);
        // std::ifstream file("C:\\Users\\jam\\source\\repos\\RawZX\\tapes\\tests\\BBG48.tap", std::ios::in | std::ios::binary);
        // std::ifstream file("C:\\Users\\jam\\source\\repos\\RawZX\\tapes\\tests\\IM0-2 (2019-11-24)(Woodmass, Mark)[!].tap", std::ios::in | std::ios::binary);
        // std::ifstream file("C:\\Users\\jam\\source\\repos\\RawZX\\tapes\\tests\\Interrupt Retriggering Test (2021-12-17)(Woodmass, Mark)[!].tap", std::ios::in | std::ios::binary);
        // std::ifstream file("C:\\Users\\jam\\source\\repos\\RawZX\\tapes\\tests\\minfo.tap", std::ios::in | std::ios::binary);
        // std::ifstream file("C:\\Users\\jam\\source\\repos\\RawZX\\tapes\\tests\\ULA 48 Simple Test (2012-10-06)(azesmbog)[!].tap", std::ios::in | std::ios::binary);
        // std::ifstream file("C:\\Users\\jam\\source\\repos\\RawZX\\tapes\\tests\\floating_bus.tap", std::ios::in | std::ios::binary);
        std::ifstream file("C:\\Users\\jam\\Documents\\Projects\\EasyZX_Deploy\\demos\\48k\\BorderTrix.tap", std::ios::in | std::ios::binary);
        // std::ifstream file("C:\\Users\\jam\\Documents\\Projects\\EasyZX_Deploy\\demos\\48k\\SHOCK.tap", std::ios::in | std::ios::binary);
        // std::ifstream file("C:\\Users\\jam\\Documents\\Projects\\EasyZX_Deploy\\demos\\128k\\BreakSpace.tap", std::ios::in | std::ios::binary);
        // std::ifstream file("C:\\Users\\jam\\Documents\\Projects\\EasyZX_Deploy\\demos\\128k\\Tiratok.tap", std::ios::in | std::ios::binary);
        // std::ifstream file("C:\\Users\\jam\\Documents\\Projects\\TESTS\\IR Contention 128 (2023-07-02)(Woodmass, Mark)[!].tap", std::ios::in | std::ios::binary);
        // std::ifstream file("C:\\Users\\jam\\Documents\\Projects\\TESTS\\ULA 128 Timing Test (2012-10-06)(azesmbog)[!].tap", std::ios::in | std::ios::binary);
        // std::ifstream file("C:\\Users\\jam\\Documents\\Projects\\TESTS\\floatspy.tap", std::ios::in | std::ios::binary);
        // std::ifstream file("C:\\Users\\jam\\Documents\\Projects\\EasyZX_Deploy\\demos\\pentagon\\ILLUSION.tap", std::ios::in | std::ios::binary);
        // std::ifstream file("C:\\Users\\jam\\Documents\\Projects\\EasyZX_Deploy\\demos\\pentagon\\7TH_REAL.tap", std::ios::in | std::ios::binary);
        // std::ifstream file("C:\\Users\\jam\\Documents\\Projects\\EasyZX_Deploy\\demos\\pentagon\\EYE_ACHE.tap", std::ios::in | std::ios::binary);
        // std::ifstream file("C:\\Users\\jam\\Documents\\Projects\\EasyZX_Deploy\\demos\\pentagon\\EYEACHE2.tap", std::ios::in | std::ios::binary);
        // std::ifstream file("C:\\Users\\jam\\Documents\\Projects\\EasyZX_Deploy\\demos\\128k\\AddMortem.tap", std::ios::in | std::ios::binary);
        // std::ifstream file("C:\\Users\\jam\\Documents\\Projects\\EasyZX_Deploy\\games\\modern\\Super Mario.tap", std::ios::in | std::ios::binary);
        // std::ifstream file("C:\\Users\\jam\\Documents\\Projects\\EasyZX_Deploy\\music\\beeper\\thevocoders.tap", std::ios::in | std::ios::binary);
        // std::ifstream file("C:\\Users\\jam\\Documents\\Projects\\EasyZX_Deploy\\music\\beeper\\EarShaver.tap", std::ios::in | std::ios::binary);
        // std::ifstream file("C:\\Users\\jam\\Documents\\Projects\\EasyZX_Deploy\\music\\beeper\\DH10Beep.tap", std::ios::in | std::ios::binary);
        // std::ifstream file("C:\\Users\\jam\\Documents\\Projects\\EasyZX_Deploy\\music\\beeper\\final_signal.tap", std::ios::in | std::ios::binary);
        if (!file.is_open())
        {
            return;
        }

        file.seekg(0, std::ios::end);
        std::streampos fileSize = file.tellg();
        tapeDataLength = static_cast<int>(fileSize);
        tapeData = new uint8_t[tapeDataLength];

        file.seekg(0, std::ios::beg);
        file.read(reinterpret_cast<char*>(tapeData), fileSize);
        file.close();
    }

    void cleanUp()
    {
        if (tapeDataLength > 0)
        {
            delete[tapeDataLength] tapeData;
            tapeData = nullptr;
            tapeDataLength = 0;
            tapeDataIndex = 0;
        }
    }

    void instantLoad()
    {
        std::span<uint8_t> block = getRawBlockData();

        if (block.empty())
        {
            return;
        }

        z80::registers.pc.w = 0x05e2;

        if (z80::registers.af_.b.h == block[0])
        {
            z80::registers.af.b.h = block[0];
            uint16_t count = block.size() - 2;
            uint16_t index = 1;

            while (z80::registers.de.w && count > 0)
            {
                z80::xorByte(block[index]);
                memory::write(z80::registers.ix.w++, block[index++]);
                --z80::registers.de.w;
                --count;
            }

            if (z80::registers.de.w)
            {
                // There are not enough bytes on the tape block
                z80::registers.af.b.l = 0x50;
            }
            else
            {
                // Successfully loaded enough bytes from tape block
                z80::xorByte(block[index]);
                z80::registers.pc.w -= 2;

                if (count)
                {
                    // TODO: There are more bytes on the tape block that some custom loaders need to read
                    //setBlockPulses();
                }
            }
        }
        else
        {
            z80::registers.af.b.l = 0;
            z80::registers.af.b.h = z80::registers.af_.b.h ^ block[0];
        }
    }
}