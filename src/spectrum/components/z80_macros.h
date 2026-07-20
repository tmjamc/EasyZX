#define FLAG_C	0x01
#define FLAG_N	0x02
#define FLAG_P	0x04
#define FLAG_V	FLAG_P
#define FLAG_3	0x08
#define FLAG_H	0x10
#define FLAG_5	0x20
#define FLAG_Z	0x40
#define FLAG_S	0x80

#define A registers.af.b.h
#define F registers.af.b.l
#define AF registers.af.w

#define B registers.bc.b.h
#define C registers.bc.b.l
#define BC registers.bc.w

#define D registers.de.b.h
#define E registers.de.b.l
#define DE registers.de.w

#define H registers.hl.b.h
#define L registers.hl.b.l
#define HL registers.hl.w

#define A_ registers.af_.b.h
#define F_ registers.af_.b.l
#define AF_ registers.af_.w

#define B_ registers.bc_.b.h
#define C_ registers.bc_.b.l
#define BC_ registers.bc_.w

#define D_ registers.de_.b.h
#define E_ registers.de_.b.l
#define DE_ registers.de_.w

#define H_ registers.hl_.b.h
#define L_ registers.hl_.b.l
#define HL_ registers.hl_.w

#define IXH registers.ix.b.h
#define IXL registers.ix.b.l
#define IX registers.ix.w

#define IYH registers.iy.b.h
#define IYL registers.iy.b.l
#define IY registers.iy.w

#define SPH registers.sp.b.h
#define SPL registers.sp.b.l
#define SP registers.sp.w

#define PCH registers.pc.b.h
#define PCL registers.pc.b.l
#define PC registers.pc.w

#define I registers.i
#define R registers.r
#define IR ((I << 8) | (R7 & 0x80) | (R & 0x7f))

#define R7 registers.r7
#define Q registers.q

#define MPH registers.memPtr.b.h
#define MPL registers.memPtr.b.l
#define MP registers.memPtr.w

#define IFF1 registers.iff1
#define IFF2 registers.iff2
#define IM registers.im

#define IS_CMOS false

#define CONTEND(addr, tacts) ula::contendedTacts(addr, tacts)
#define CONTEND_READ_NO_MREQ(addr, tacts) ula::contendedTacts(addr, tacts)
#define CONTEND_WRITE_NO_MREQ(addr, tacts) ula::contendedTacts(addr, tacts)

#define INC(r)                                                                          \
    ++r;                                                                                \
    F = (F & FLAG_C) | (r == 0x80 ? FLAG_V : 0) | (r & 0x0f ? 0 : FLAG_H) | sz53[(r)]; \
    Q = F

#define DEC(r)                                           \
    F = (F & FLAG_C) | (r & 0x0f ? 0 : FLAG_H) | FLAG_N; \
    --r;                                                 \
    F |= (r == 0x7f ? FLAG_V : 0) | sz53[r];            \
    Q = F

#define AND(r)              \
    A &= r;                 \
    F = FLAG_H | sz53p[A]; \
    Q = F

#define XOR(r)     \
    A ^= r;        \
    F = sz53p[A]; \
    Q = F

#define OR(r)      \
    A |= r;        \
    F = sz53p[A]; \
    Q = F

#define RET()       \
    POPW(PCL, PCH); \
    MP = PC

#define POPW(rl, rh)     \
    rl = contendRead(SP++); \
    rh = contendRead(SP++)

#define PUSHW(rl, rh)   \
    contendWrite(--SP, rh); \
    contendWrite(--SP, rl)

#define CALL()                   \
    CONTEND_READ_NO_MREQ(PC, 1); \
    ++PC;                        \
    PUSHW(PCL, PCH);             \
    PC = MP

#define RST(r)       \
    PUSHW(PCL, PCH); \
    PC = r;          \
    MP = PC

#define Z80_IN(r, port)               \
    {                                 \
        MP = port + 1;                \
        r = io::read(port);      \
        F = (F & FLAG_C) | sz53p[r]; \
        Q = F;                        \
    }

#define CP(r)                                                                                                                                                                \
    {                                                                                                                                                                        \
        uint16_t temp = A - r;                                                                                                                                               \
        uint8_t lookup = ((A & 0x88) >> 3) | ((r & 0x88) >> 2) | ((temp & 0x88) >> 1);                                                                                       \
        F = ((temp & 0x100) ? FLAG_C : (temp ? 0 : FLAG_Z)) | FLAG_N | halfcarrySub[lookup & 0x07] | overflowSub[lookup >> 4] | (r & (FLAG_3 | FLAG_5)) | (temp & FLAG_S); \
        Q = F;                                                                                                                                                               \
    }

#define ADD(r)                                                                                                   \
    {                                                                                                            \
        uint16_t temp = A + r;                                                                                   \
        uint8_t lookup = ((A & 0x88) >> 3) | ((r & 0x88) >> 2) | ((temp & 0x88) >> 1);                           \
        A = temp;                                                                                                \
        F = ((temp & 0x100) ? FLAG_C : 0) | halfcarryAdd[lookup & 0x07] | overflowAdd[lookup >> 4] | sz53[A]; \
        Q = F;                                                                                                   \
    }

#define ADC(r)                                                                                                   \
    {                                                                                                            \
        uint16_t temp = A + r + (F & FLAG_C);                                                                    \
        uint8_t lookup = ((A & 0x88) >> 3) | ((r & 0x88) >> 2) | ((temp & 0x88) >> 1);                           \
        A = temp;                                                                                                \
        F = ((temp & 0x100) ? FLAG_C : 0) | halfcarryAdd[lookup & 0x07] | overflowAdd[lookup >> 4] | sz53[A]; \
        Q = F;                                                                                                   \
    }

#define ADDW(r1, r2)                                                                                                                        \
    {                                                                                                                                       \
        uint32_t temp = r1 + r2;                                                                                                            \
        uint8_t lookup = ((r1 & 0x0800) >> 11) | ((r2 & 0x0800) >> 10) | ((temp & 0x0800) >> 9);                                            \
        MP = r1 + 1;                                                                                                                        \
        r1 = temp;                                                                                                                          \
        F = (F & (FLAG_V | FLAG_Z | FLAG_S)) | ((temp & 0x10000) ? FLAG_C : 0) | ((temp >> 8) & (FLAG_3 | FLAG_5)) | halfcarryAdd[lookup]; \
        Q = F;                                                                                                                              \
    }

#define ADCW(r)                                                                                                                                                \
    {                                                                                                                                                          \
        uint32_t temp = HL + r + (F & FLAG_C);                                                                                                                 \
        uint8_t lookup = ((HL & 0x8800) >> 11) | ((r & 0x8800) >> 10) | ((temp & 0x8800) >> 9);                                                                \
        MP = HL + 1;                                                                                                                                           \
        HL = temp;                                                                                                                                             \
        F = ((temp & 0x10000) ? FLAG_C : 0) | overflowAdd[lookup >> 4] | (H & (FLAG_3 | FLAG_5 | FLAG_S)) | halfcarryAdd[lookup & 0x07] | (HL ? 0 : FLAG_Z); \
        Q = F;                                                                                                                                                 \
    }

#define SUB(r)                                                                                                            \
    {                                                                                                                     \
        uint16_t temp = A - r;                                                                                            \
        uint8_t lookup = ((A & 0x88) >> 3) | ((r & 0x88) >> 2) | ((temp & 0x88) >> 1);                                    \
        A = temp;                                                                                                         \
        F = ((temp & 0x100) ? FLAG_C : 0) | FLAG_N | halfcarrySub[lookup & 0x07] | overflowSub[lookup >> 4] | sz53[A]; \
        Q = F;                                                                                                            \
    }

#define SBC(r)                                                                                                            \
    {                                                                                                                     \
        uint16_t temp = A - r - (F & FLAG_C);                                                                             \
        uint8_t lookup = ((A & 0x88) >> 3) | ((r & 0x88) >> 2) | ((temp & 0x88) >> 1);                                    \
        A = temp;                                                                                                         \
        F = ((temp & 0x100) ? FLAG_C : 0) | FLAG_N | halfcarrySub[lookup & 0x07] | overflowSub[lookup >> 4] | sz53[A]; \
        Q = F;                                                                                                            \
    }

#define SBCW(r)                                                                                                                                                         \
    {                                                                                                                                                                   \
        uint32_t temp = HL - r - (F & FLAG_C);                                                                                                                          \
        uint8_t lookup = ((HL & 0x8800) >> 11) | ((r & 0x8800) >> 10) | ((temp & 0x8800) >> 9);                                                                         \
        MP = HL + 1;                                                                                                                                                    \
        HL = temp;                                                                                                                                                      \
        F = ((temp & 0x10000) ? FLAG_C : 0) | FLAG_N | overflowSub[lookup >> 4] | (H & (FLAG_3 | FLAG_5 | FLAG_S)) | halfcarrySub[lookup & 0x07] | (HL ? 0 : FLAG_Z); \
        Q = F;                                                                                                                                                          \
    }

#define JP() PC = MP

#define JR()                         \
    {                                \
        int8_t temp = contendRead(PC);  \
        CONTEND_READ_NO_MREQ(PC, 1); \
        CONTEND_READ_NO_MREQ(PC, 1); \
        CONTEND_READ_NO_MREQ(PC, 1); \
        CONTEND_READ_NO_MREQ(PC, 1); \
        CONTEND_READ_NO_MREQ(PC, 1); \
        PC += temp;                  \
        ++PC;                        \
        MP = PC;                     \
    }

#define LDW_NNRR(rl, rh)             \
    {                                \
        uint16_t temp;               \
        temp = contendRead(PC++);       \
        temp |= contendRead(PC++) << 8; \
        contendWrite(temp++, rl);        \
        MP = temp;                   \
        contendWrite(temp, rh);          \
    }

#define LDW_RRNN(rl, rh)             \
    {                                \
        uint16_t temp;               \
        temp = contendRead(PC++);       \
        temp |= contendRead(PC++) << 8; \
        rl = contendRead(temp++);       \
        MP = temp;                   \
        rh = contendRead(temp);         \
    }

#define RLC(r)                    \
    r = (r << 1) | (r >> 7);      \
    F = (r & FLAG_C) | sz53p[r]; \
    Q = F

#define RRC(r)               \
    F = r & FLAG_C;          \
    r = (r >> 1) | (r << 7); \
    F |= sz53p[r];          \
    Q = F

#define RL(r)                        \
    {                                \
        uint8_t temp = r;            \
        r = (r << 1) | (F & FLAG_C); \
        F = (temp >> 7) | sz53p[r]; \
        Q = F;                       \
    }

#define RR(r)                            \
    {                                    \
        uint8_t temp = r;                \
        r = (r >> 1) | (F << 7);         \
        F = (temp & FLAG_C) | sz53p[r]; \
        Q = F;                           \
    }

#define SLA(r)      \
    F = r >> 7;     \
    r <<= 1;        \
    F |= sz53p[r]; \
    Q = F

#define SRA(r)                 \
    F = r & FLAG_C;            \
    r = (r & 0x80) | (r >> 1); \
    F |= sz53p[r];            \
    Q = F

#define SLL(r)           \
    F = r >> 7;          \
    r = (r << 1) | 0x01; \
    F |= sz53p[r];      \
    Q = F

#define SRL(r)      \
    F = r & FLAG_C; \
    r >>= 1;        \
    F |= sz53p[r]; \
    Q = F

#define BIT(bit, r)                                      \
    F = (F & FLAG_C) | FLAG_H | (r & (FLAG_3 | FLAG_5)); \
    if (!(r & (0x01 << bit)))                            \
        F |= FLAG_P | FLAG_Z;                            \
    if (bit == 7 && r & 0x80)                            \
        F |= FLAG_S;                                     \
    Q = F

#define BIT_MP(bit, r)                                     \
    F = (F & FLAG_C) | FLAG_H | (MPH & (FLAG_3 | FLAG_5)); \
    if (!(r & (0x01 << bit)))                              \
        F |= FLAG_P | FLAG_Z;                              \
    if (bit == 7 && r & 0x80)                              \
        F |= FLAG_S;                                       \
    Q = F

#define REP_2(r) \
    r;           \
    r

#define REP_3(r) \
    r;           \
    r;           \
    r

#define REP_4(r) \
    r;           \
    r;           \
    r;           \
    r

#define REP_5(r) \
    r;           \
    r;           \
    r;           \
    r;           \
    r

#define REP_6(r) \
    r;           \
    r;           \
    r;           \
    r;           \
    r;           \
    r

#define REP_7(r) \
    r;           \
    r;           \
    r;           \
    r;           \
    r;           \
    r;           \
    r