#include <span>
#include <fstream>

#include "tape.h"
#include "z80.h"
#include "memory.h"
#include "main.h"

namespace tape
{
    // int tapeDataLength = 0;
    // int tapeDataIndex = 0;
    // uint8_t* tapeData = nullptr;

    struct eTapeState
    {
        uint64_t edge_change;
        uint8_t *play_pointer; // or NULL if tape stopped
        uint8_t *end_of_tape;  // where to stop tape
        uint32_t index;        // current tape block
        uint32_t tape_bit;
    };
    eTapeState tape;

    struct TAPEINFO
    {
        char desc[280];
        uint32_t pos;
        uint32_t t_size;
    };

    uint32_t tape_pulse[0x100];
    uint32_t max_pulses;
    uint32_t tape_err;

    uint8_t *tape_image;
    uint32_t tape_imagesize;

    TAPEINFO *tapeinfo;
    uint32_t tape_infosize;

    uint32_t appendable;

    /*
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
    */

/*
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
    */

    // void Init()
    // {
    //     max_pulses = 0;
    //     tape_err = 0;

    //     tape_image = NULL;
    //     tape_imagesize = 0;

    //     tapeinfo = NULL;
    //     tape_infosize = 0;

    //     appendable = 0;
    // }

    static inline uint16_t Word(const uint8_t* ptr)	{ return ptr[0] | uint16_t(ptr[1]) << 8; }
    static inline uint32_t Dword(const uint8_t* ptr)
    {
        return ptr[0] | uint32_t(ptr[1]) << 8 | uint32_t(ptr[2]) << 16 | uint32_t(ptr[3]) << 24;
    }


    bool Started()
    {
        return tape.play_pointer != NULL;
    }

    bool Inserted()
    {
        return tape_image != NULL;
    }

    // bool IoRead(uint16_t port)
    // {
    //     return !(port & 1);
    // }

    // void IoRead(uint16_t port, uint8_t *v/*, int tact*/)
    // {
    //     *v |= TapeBit(/*tact*/) & 0x40;
    // }

    uint32_t FindPulse(uint32_t t)
    {
        if (max_pulses < 0x100)
        {
            for (uint32_t i = 0; i < max_pulses; i++)
                if (tape_pulse[i] == t)
                    return i;
            tape_pulse[max_pulses] = t;
            return max_pulses++;
        }
        tape_err = 1;
        uint32_t nearest = 0;
        int delta = 0x7FFFFFFF;
        for (uint32_t i = 0; i < 0x100; i++)
            if (delta > abs((int)t - (int)tape_pulse[i]))
                nearest = i, delta = abs((int)t - (int)tape_pulse[i]);
        return nearest;
    }

    void FindTapeIndex()
    {
        for (uint32_t i = 0; i < tape_infosize; i++)
            if (tape.play_pointer >= tape_image + tapeinfo[i].pos)
                tape.index = i;
    }

    void FindTapeSizes()
    {
        for (uint32_t i = 0; i < tape_infosize; i++)
        {
            uint32_t end = (i == tape_infosize - 1) ? tape_imagesize
                                                 : tapeinfo[i + 1].pos;
            uint32_t sz = 0;
            for (uint32_t j = tapeinfo[i].pos; j < end; j++)
                sz += tape_pulse[tape_image[j]];
            tapeinfo[i].t_size = sz;
        }
    }

    void StopTape()
    {
        FindTapeIndex();
        if (tape.play_pointer >= tape.end_of_tape)
            tape.index = 0;
        tape.play_pointer = 0;
        tape.edge_change = 0x7FFFFFFFFFFFFFFFLL;
        tape.tape_bit = -1;
        // speccy->CPU()->HandlerStep(NULL);
    }

    void ResetTape()
    {
        tape.index = 0;
        tape.play_pointer = 0;
        tape.edge_change = 0x7FFFFFFFFFFFFFFFLL;
        tape.tape_bit = -1;
        // speccy->CPU()->HandlerStep(NULL);
    }

    void StartTape()
    {
        if (!tape_image)
            return;
        tape.play_pointer = tape_image + tapeinfo[tape.index].pos;
        tape.end_of_tape = tape_image + tape_imagesize;
        tape.edge_change = main::totalTacts;
        tape.tape_bit = -1;
        //	speccy->CPU()->FastEmul(FastTapeEmul);
    }

    void CloseTape()
    {
        if (tape_image)
        {
            free(tape_image);
            tape_image = 0;
        }
        if (tapeinfo)
        {
            free(tapeinfo);
            tapeinfo = 0;
        }
        tape.play_pointer = 0; // stop tape
        tape.index = 0;        // rewind tape
        tape_err = max_pulses = tape_imagesize = tape_infosize = 0;
        tape.edge_change = 0x7FFFFFFFFFFFFFFFLL;
        tape.tape_bit = -1;
    }

#define align_by(a, b) (((uint32_t)(a) + ((b) - 1)) & ~((b) - 1))

    void Reserve(uint32_t datasize)
    {
        const int blocksize = 16384;
        uint32_t newsize = align_by(datasize + tape_imagesize + 1, blocksize);
        if (!tape_image)
            tape_image = (uint8_t *)malloc(newsize);
        if (align_by(tape_imagesize, blocksize) < newsize)
            tape_image = (uint8_t *)realloc(tape_image, newsize);
    }

    void MakeBlock(const uint8_t *data, uint32_t size, uint32_t pilot_t, uint32_t s1_t,
                          uint32_t s2_t, uint32_t zero_t, uint32_t one_t, uint32_t pilot_len, uint32_t pause,
                          uint8_t last = 8)
    {
        Reserve(size * 16 + pilot_len + 3);
        if (pilot_len != (uint32_t)-1)
        {
            uint32_t t = FindPulse(pilot_t);
            for (uint32_t i = 0; i < pilot_len; i++)
                tape_image[tape_imagesize++] = t;
            tape_image[tape_imagesize++] = FindPulse(s1_t);
            tape_image[tape_imagesize++] = FindPulse(s2_t);
        }
        uint32_t t0 = FindPulse(zero_t), t1 = FindPulse(one_t);
        for (; size > 1; size--, data++)
            for (uint8_t j = 0x80; j; j >>= 1)
                tape_image[tape_imagesize++] = (*data & j) ? t1 : t0, tape_image[tape_imagesize++] = (*data & j) ? t1 : t0;
        for (uint8_t j = 0x80; j != (uint8_t)(0x80 >> last); j >>= 1) // last uint8_t
            tape_image[tape_imagesize++] = (*data & j) ? t1 : t0, tape_image[tape_imagesize++] = (*data & j) ? t1 : t0;
        if (pause)
            tape_image[tape_imagesize++] = FindPulse(pause * 3500);
    }

    void Desc(const uint8_t *data, uint32_t size, char *dst)
    {
        uint8_t crc = 0;
        char prg[10];
        uint32_t i; // Alone Coder 0.36.7
        for (/*uint32_t*/ i = 0; i < size; i++)
            crc ^= data[i];
        if (!*data && size == 19 && (data[1] == 0 || data[1] == 3))
        {
            for (i = 0; i < 10; i++)
                prg[i] = (data[i + 2] < ' ' || data[i + 2] >= 0x80) ? '?' : data[i + 2];
            for (i = 9; i && prg[i] == ' '; prg[i--] = 0)
                ;
            sprintf(dst, "%s: \"%s\" %d,%d", data[1] ? "uint8_ts" : "Program", prg,
                    Word(data + 14), Word(data + 12));
        }
        else if (*data == 0xFF)
            sprintf(dst, "data block, %d uint8_ts", size - 2);
        else
            sprintf(dst, "#%02X block, %d uint8_ts", *data, size - 2);
        sprintf(dst + strlen(dst), ", crc %s", crc ? "bad" : "ok");
    }

    void AllocInfocell()
    {
        tapeinfo = (TAPEINFO *)realloc(tapeinfo, (tape_infosize + 1) * sizeof(TAPEINFO));
        tapeinfo[tape_infosize].pos = tape_imagesize;
        appendable = 0;
    }

    void NamedCell(const void *nm, uint32_t sz = 0)
    {
        AllocInfocell();
        if (sz)
            memcpy(tapeinfo[tape_infosize].desc, nm, sz), tapeinfo[tape_infosize].desc[sz] = 0;
        else
            strcpy(tapeinfo[tape_infosize].desc, (const char *)nm);
        tape_infosize++;
    }

    bool ParseTAP(const void *data, size_t data_size)
    {
        const uint8_t *ptr = (const uint8_t *)data;
        CloseTape();
        while (ptr < (const uint8_t *)data + data_size)
        {
            uint32_t size = Word(ptr);
            ptr += 2;
            if (!size)
                break;
            AllocInfocell();
            Desc(ptr, size, tapeinfo[tape_infosize].desc);
            tape_infosize++;
            MakeBlock(ptr, size, 2168, 667, 735, 855, 1710, (*ptr < 4) ? 8064 : 3220, 1000);
            ptr += size;
        }
        FindTapeSizes();
        return (ptr == (const uint8_t *)data + data_size);
    }

    bool ParseCSW(const void *data, size_t data_size)
    {
        const uint8_t *buf = (const uint8_t *)data;
        const uint32_t Z80FQ = 3500000;
        CloseTape();
        NamedCell("CSW tape image");
        if (buf[0x1B] != 1)
            return false;                      // unknown compression type
        uint32_t rate = Z80FQ / Word(buf + 0x19); // usually 3.5mhz / 44khz
        if (!rate)
            return false;
        Reserve(data_size - 0x18);
        if (!(buf[0x1C] & 1))
            tape_image[tape_imagesize++] = FindPulse(1);
        for (const uint8_t *ptr = (const uint8_t *)data + 0x20; ptr < (const uint8_t *)data + data_size;)
        {
            uint32_t len = *ptr++ * rate;
            if (!len)
            {
                len = Dword(ptr) / rate;
                ptr += 4;
            }
            tape_image[tape_imagesize++] = FindPulse(len);
        }
        tape_image[tape_imagesize++] = FindPulse(Z80FQ / 10);
        FindTapeSizes();
        return true;
    }

    void CreateAppendableBlock()
    {
        if (!tape_infosize || appendable)
            return;
        NamedCell("set of pulses");
        appendable = 1;
    }
    
    void ParseHardware(const uint8_t *ptr)
    {
        uint32_t n = *ptr++;
        if (!n)
            return;
        NamedCell("- HARDWARE TYPE ");
        static const char ids[] = "computer\0"
                                  "ZX Spectrum 16k\0"
                                  "ZX Spectrum 48k, Plus\0"
                                  "ZX Spectrum 48k ISSUE 1\0"
                                  "ZX Spectrum 128k (Sinclair)\0"
                                  "ZX Spectrum 128k +2 (Grey case)\0"
                                  "ZX Spectrum 128k +2A, +3\0"
                                  "Timex Sinclair TC-2048\0"
                                  "Timex Sinclair TS-2068\0"
                                  "Pentagon 128\0"
                                  "Sam Coupe\0"
                                  "Didaktik M\0"
                                  "Didaktik Gama\0"
                                  "ZX-81 or TS-1000 with  1k RAM\0"
                                  "ZX-81 or TS-1000 with 16k RAM or more\0"
                                  "ZX Spectrum 128k, Spanish version\0"
                                  "ZX Spectrum, Arabic version\0"
                                  "TK 90-X\0"
                                  "TK 95\0"
                                  "Byte\0"
                                  "Elwro\0"
                                  "ZS Scorpion\0"
                                  "Amstrad CPC 464\0"
                                  "Amstrad CPC 664\0"
                                  "Amstrad CPC 6128\0"
                                  "Amstrad CPC 464+\0"
                                  "Amstrad CPC 6128+\0"
                                  "Jupiter ACE\0"
                                  "Enterprise\0"
                                  "Commodore 64\0"
                                  "Commodore 128\0"
                                  "\0"
                                  "ext. storage\0"
                                  "Microdrive\0"
                                  "Opus Discovery\0"
                                  "Disciple\0"
                                  "Plus-D\0"
                                  "Rotronics Wafadrive\0"
                                  "TR-DOS (BetaDisk)\0"
                                  "Byte Drive\0"
                                  "Watsford\0"
                                  "FIZ\0"
                                  "Radofin\0"
                                  "Didaktik disk drives\0"
                                  "BS-DOS (MB-02)\0"
                                  "ZX Spectrum +3 disk drive\0"
                                  "JLO (Oliger) disk interface\0"
                                  "FDD3000\0"
                                  "Zebra disk drive\0"
                                  "Ramex Millenia\0"
                                  "Larken\0"
                                  "\0"
                                  "ROM/RAM type add-on\0"
                                  "Sam Ram\0"
                                  "Multiface\0"
                                  "Multiface 128k\0"
                                  "Multiface +3\0"
                                  "MultiPrint\0"
                                  "MB-02 ROM/RAM expansion\0"
                                  "\0"
                                  "sound device\0"
                                  "Classic AY hardware\0"
                                  "Fuller Box AY sound hardware\0"
                                  "Currah microSpeech\0"
                                  "SpecDrum\0"
                                  "AY ACB stereo; Melodik\0"
                                  "AY ABC stereo\0"
                                  "\0"
                                  "joystick\0"
                                  "Kempston\0"
                                  "Cursor, Protek, AGF\0"
                                  "Sinclair 2\0"
                                  "Sinclair 1\0"
                                  "Fuller\0"
                                  "\0"
                                  "mice\0"
                                  "AMX mouse\0"
                                  "Kempston mouse\0"
                                  "\0"
                                  "other controller\0"
                                  "Trickstick\0"
                                  "ZX Light Gun\0"
                                  "Zebra Graphics Tablet\0"
                                  "\0"
                                  "serial port\0"
                                  "ZX Interface 1\0"
                                  "ZX Spectrum 128k\0"
                                  "\0"
                                  "parallel port\0"
                                  "Kempston S\0"
                                  "Kempston E\0"
                                  "ZX Spectrum 128k +2A, +3\0"
                                  "Tasman\0"
                                  "DK'Tronics\0"
                                  "Hilderbay\0"
                                  "INES Printerface\0"
                                  "ZX LPrint Interface 3\0"
                                  "MultiPrint\0"
                                  "Opus Discovery\0"
                                  "Standard 8255 chip with ports 31,63,95\0"
                                  "\0"
                                  "printer\0"
                                  "ZX Printer, Alphacom 32 & compatibles\0"
                                  "Generic Printer\0"
                                  "EPSON Compatible\0"
                                  "\0"
                                  "modem\0"
                                  "VTX 5000\0"
                                  "T/S 2050 or Westridge 2050\0"
                                  "\0"
                                  "digitaiser\0"
                                  "RD Digital Tracer\0"
                                  "DK'Tronics Light Pen\0"
                                  "British MicroGraph Pad\0"
                                  "\0"
                                  "network adapter\0"
                                  "ZX Interface 1\0"
                                  "\0"
                                  "keyboard / keypad\0"
                                  "Keypad for ZX Spectrum 128k\0"
                                  "\0"
                                  "AD/DA converter\0"
                                  "Harley Systems ADC 8.2\0"
                                  "Blackboard Electronics\0"
                                  "\0"
                                  "EPROM Programmer\0"
                                  "Orme Electronics\0"
                                  "\0"
                                  "\0";
        for (uint32_t i = 0; i < n; i++)
        {
            uint8_t type_n = *ptr++;
            uint8_t id_n = *ptr++;
            uint8_t value_n = *ptr++;
            const char *type = ids, *id, *value;
            uint32_t j; // Alone Coder 0.36.7
            for (/*uint32_t*/ j = 0; j < type_n; j++)
            {
                if (!*type)
                    break;
                while (Word((uint8_t *)type))
                    type++;
                type += 2;
            }
            if (!*type)
                type = id = "??";
            else
            {
                id = type + strlen(type) + 1;
                for (j = 0; j < id_n; j++)
                {
                    if (!*id)
                    {
                        id = "??";
                        break;
                    }
                    id += strlen(id) + 1;
                }
            }
            switch (value_n)
            {
            case 0:
                value = "compatible with";
                break;
            case 1:
                value = "uses";
                break;
            case 2:
                value = "compatible, but doesn't use";
                break;
            case 3:
                value = "incompatible with";
                break;
            default:
                value = "??";
            }
            char bf[512];
            sprintf(bf, "%s %s: %s", value, type, id);
            NamedCell(bf);
        }
        NamedCell("-");
    }

    bool ParseTZX(const void *data, size_t data_size)
    {
        uint8_t *ptr = (uint8_t *)data;
        CloseTape();
        uint32_t size, pause, i, j, n, t, t0;
        uint8_t pl, last, *end;
        uint8_t *p;
        uint32_t loop_n = 0, loop_p = 0;
        char nm[512];
        while (ptr < (const uint8_t *)data + data_size)
        {
            switch (*ptr++)
            {
            case 0x10: // normal block
                AllocInfocell();
                size = Word(ptr + 2);
                pause = Word(ptr);
                ptr += 4;
                Desc(ptr, size, tapeinfo[tape_infosize].desc);
                tape_infosize++;
                MakeBlock(ptr, size, 2168, 667, 735, 855, 1710, (*ptr < 4) ? 8064 : 3220, pause);
                ptr += size;
                break;
            case 0x11: // turbo block
                AllocInfocell();
                size = 0xFFFFFF & Dword(ptr + 0x0F);
                Desc(ptr + 0x12, size, tapeinfo[tape_infosize].desc);
                tape_infosize++;
                MakeBlock(ptr + 0x12, size, Word(ptr), Word(ptr + 2),
                          Word(ptr + 4), Word(ptr + 6), Word(ptr + 8),
                          Word(ptr + 10), Word(ptr + 13), ptr[12]);
                // todo: test used bits - ptr+12
                ptr += size + 0x12;
                break;
            case 0x12: // pure tone
                CreateAppendableBlock();
                pl = FindPulse(Word(ptr));
                n = Word(ptr + 2);
                Reserve(n);
                for (i = 0; i < n; i++)
                    tape_image[tape_imagesize++] = pl;
                ptr += 4;
                break;
            case 0x13: // sequence of pulses of different lengths
                CreateAppendableBlock();
                n = *ptr++;
                Reserve(n);
                for (i = 0; i < n; i++, ptr += 2)
                    tape_image[tape_imagesize++] = FindPulse(Word(ptr));
                break;
            case 0x14: // pure data block
                CreateAppendableBlock();
                size = 0xFFFFFF & Dword(ptr + 7);
                MakeBlock(ptr + 0x0A, size, 0, 0, 0, Word(ptr),
                          Word(ptr + 2), -1, Word(ptr + 5), ptr[4]);
                ptr += size + 0x0A;
                break;
            case 0x15: // direct recording
                size = 0xFFFFFF & Dword(ptr + 5);
                t0 = Word(ptr);
                pause = Word(ptr + 2);
                last = ptr[4];
                NamedCell("direct recording");
                ptr += 8;
                pl = 0;
                n = 0;
                for (i = 0; i < size; i++) // count number of pulses
                    for (j = 0x80; j; j >>= 1)
                        if ((ptr[i] ^ pl) & j)
                            n++, pl ^= -1;
                t = 0;
                pl = 0;
                Reserve(n + 2);
                for (i = 1; i < size; i++, ptr++) // find pulses
                    for (j = 0x80; j; j >>= 1)
                    {
                        t += t0;
                        if ((*ptr ^ pl) & j)
                        {
                            tape_image[tape_imagesize++] = FindPulse(t);
                            pl ^= -1;
                            t = 0;
                        }
                    }
                // find pulses - last uint8_t
                for (j = 0x80; j != (uint8_t)(0x80 >> last); j >>= 1)
                {
                    t += t0;
                    if ((*ptr ^ pl) & j)
                    {
                        tape_image[tape_imagesize++] = FindPulse(t);
                        pl ^= -1;
                        t = 0;
                    }
                }
                ptr++;
                tape_image[tape_imagesize++] = FindPulse(t); // last pulse ???
                if (pause)
                    tape_image[tape_imagesize++] = FindPulse(pause * 3500);
                break;
            case 0x20: // pause (silence) or 'stop the tape' command
                pause = Word(ptr);
                sprintf(nm, pause ? "pause %d ms" : "stop the tape", pause);
                NamedCell(nm);
                Reserve(2);
                ptr += 2;
                if (!pause)
                { // at least 1ms pulse as specified in TZX 1.13
                    tape_image[tape_imagesize++] = FindPulse(3500);
                    pause = -1;
                }
                else
                    pause *= 3500;
                tape_image[tape_imagesize++] = FindPulse(pause);
                break;
            case 0x21: // group start
                n = *ptr++;
                NamedCell(ptr, n);
                ptr += n;
                appendable = 1;
                break;
            case 0x22: // group end
                break;
            case 0x23: // jump to block
                NamedCell("* jump");
                ptr += 2;
                break;
            case 0x24: // loop start
                loop_n = Word(ptr);
                loop_p = tape_imagesize;
                ptr += 2;
                break;
            case 0x25: // loop end
                if (!loop_n)
                    break;
                size = tape_imagesize - loop_p;
                Reserve((loop_n - 1) * size);
                for (i = 1; i < loop_n; i++)
                    memcpy(tape_image + loop_p + i * size, tape_image + loop_p,
                           size);
                tape_imagesize += (loop_n - 1) * size;
                loop_n = 0;
                break;
            case 0x26: // call
                NamedCell("* call");
                ptr += 2 + 2 * Word(ptr);
                break;
            case 0x27: // ret
                NamedCell("* return");
                break;
            case 0x28: // select block
                sprintf(nm, "* choice: ");
                n = ptr[2];
                p = ptr + 3;
                for (i = 0; i < n; i++)
                {
                    if (i)
                        strcat(nm, " / ");
                    char *q = nm + strlen(nm);
                    size = *(p + 2);
                    memcpy(q, p + 3, size);
                    q[size] = 0;
                    p += size + 3;
                }
                NamedCell(nm);
                ptr += 2 + Word(ptr);
                break;
            case 0x2A: // stop if 48k
                NamedCell("* stop if 48K");
                ptr += 4 + Dword(ptr);
                break;
            case 0x30: // text description
                n = *ptr++;
                NamedCell(ptr, n);
                ptr += n;
                appendable = 1;
                break;
            case 0x31: // message block
                NamedCell("- MESSAGE BLOCK ");
                end = ptr + 2 + ptr[1];
                pl = *end;
                *end = 0;
                for (p = ptr + 2; p < end; p++)
                    if (*p == 0x0D)
                        *p = 0;
                for (p = ptr + 2; p < end; p += strlen((char *)p) + 1)
                    NamedCell(p);
                *end = pl;
                ptr = end;
                NamedCell("-");
                break;
            case 0x32: // archive info
                NamedCell("- ARCHIVE INFO ");
                p = ptr + 3;
                for (i = 0; i < ptr[2]; i++)
                {
                    const char *info;
                    switch (*p++)
                    {
                    case 0:
                        info = "Title";
                        break;
                    case 1:
                        info = "Publisher";
                        break;
                    case 2:
                        info = "Author";
                        break;
                    case 3:
                        info = "Year";
                        break;
                    case 4:
                        info = "Language";
                        break;
                    case 5:
                        info = "Type";
                        break;
                    case 6:
                        info = "Price";
                        break;
                    case 7:
                        info = "Protection";
                        break;
                    case 8:
                        info = "Origin";
                        break;
                    case 0xFF:
                        info = "Comment";
                        break;
                    default:
                        info = "info";
                        break;
                    }
                    uint32_t size = *p + 1;
                    char tmp = p[size];
                    p[size] = 0;
                    sprintf(nm, "%s: %s", info, p + 1);
                    p[size] = tmp;
                    p += size;
                    NamedCell(nm);
                }
                NamedCell("-");
                ptr += 2 + Word(ptr);
                break;
            case 0x33: // hardware type
                ParseHardware(ptr);
                ptr += 1 + 3 * *ptr;
                break;
            case 0x34: // emulation info
                NamedCell("* emulation info");
                ptr += 8;
                break;
            case 0x35: // custom info
                if (!memcmp(ptr, "POKEs           ", 16))
                {
                    NamedCell("- POKEs block ");
                    NamedCell(ptr + 0x15, ptr[0x14]);
                    p = ptr + 0x15 + ptr[0x14];
                    n = *p++;
                    for (i = 0; i < n; i++)
                    {
                        NamedCell(p + 1, *p);
                        p += *p + 1;
                        t = *p++;
                        strcpy(nm, "POKE ");
                        for (j = 0; j < t; j++)
                        {
                            sprintf(nm + strlen(nm), "%d,", Word(p + 1));
                            sprintf(nm + strlen(nm), *p & 0x10 ? "nn" : "%d",
                                    *(uint8_t *)(p + 3));
                            if (!(*p & 0x08))
                                sprintf(nm + strlen(nm), "(page %d)", *p & 7);
                            strcat(nm, "; ");
                            p += 5;
                        }
                        NamedCell(nm);
                    }
                    nm[0] = '-';
                    nm[1] = 0;
                    nm[2] = 0;
                    nm[3] = 0;
                }
                else
                    sprintf(nm, "* custom info: %s", ptr), nm[15 + 16] = 0;
                NamedCell(nm);
                ptr += 0x14 + Dword(ptr + 0x10);
                break;
            case 0x40: // snapshot
                NamedCell("* snapshot");
                ptr += 4 + (0xFFFFFF & Dword(ptr + 1));
                break;
            case 0x5A: // 'Z'
                ptr += 9;
                break;
            default:
                ptr += data_size;
            }
        }
        for (i = 0; i < tape_infosize; i++)
        {
            if (tapeinfo[i].desc[0] == '*' && tapeinfo[i].desc[1] == ' ')
                strcat(tapeinfo[i].desc, " [UNSUPPORTED]");
            if (*tapeinfo[i].desc == '-')
                while (strlen(tapeinfo[i].desc) < sizeof(tapeinfo[i].desc) - 1)
                    strcat(tapeinfo[i].desc, "-");
        }
        if (tape_imagesize && tape_pulse[tape_image[tape_imagesize - 1]] < 350000)
            Reserve(1), tape_image[tape_imagesize++] = FindPulse(350000); // small pause [rqd for 3ddeathchase]
        FindTapeSizes();
        return (ptr == (const uint8_t *)data + data_size);
    }

    bool Open(const char *type, const void *data, size_t data_size)
    {
        if (!strcmp(type, "tap"))
            return ParseTAP(data, data_size);
        else if (!strcmp(type, "csw"))
            return ParseCSW(data, data_size);
        else if (!strcmp(type, "tzx"))
            return ParseTZX(data, data_size);
        return false;
    }

    // uint8_t TapeBit(int tact)
    uint8_t TapeBit()
    {
        while (main::totalTacts > tape.edge_change)
        {
            // uint32_t t = (uint32_t)(main::currentTact - tape.edge_change);
            // if ((int)t >= 0)
            // {
            //     const short vol = 1000;
            //     short mono = tape.tape_bit ? vol : 0;
            //     // Update(tact, mono, mono);
            // }
            uint32_t pulse;
            tape.tape_bit ^= -1;
            if (tape.play_pointer >= tape.end_of_tape ||
                (pulse = tape_pulse[*tape.play_pointer++]) == (uint32_t)-1)
                StopTape();
            else
                tape.edge_change += pulse;
        }
        return (uint8_t)tape.tape_bit;
    }

    void init()
    {
        max_pulses = 0;
        tape_err = 0;

        tape_image = NULL;
        tape_imagesize = 0;

        tapeinfo = NULL;
        tape_infosize = 0;

        appendable = 0;

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
        // std::ifstream file("C:\\Users\\jam\\Documents\\Projects\\EasyZX_Deploy\\demos\\128k\\Ganzfeld(Final).tap", std::ios::in | std::ios::binary);
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
        char* buffer = new char[fileSize];

        file.seekg(0, std::ios::beg);
        file.read(buffer, fileSize);
        file.close();

        ParseTAP(buffer, fileSize);
    }

    void cleanUp()
    {
        CloseTape();
        // if (tape_imagesize > 0)
        // {
        //     delete[tapeDataLength] tapeData;
        //     tapeData = nullptr;
        //     tapeDataLength = 0;
        //     // tapeDataIndex = 0;
        // }
    }
}