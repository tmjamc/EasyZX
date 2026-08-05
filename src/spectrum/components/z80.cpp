#include "win_app.h"
#include "z80.h"
#include "memory.h"
#include "ula.h"
#include "io.h"
#include "z80_macros.h"

namespace z80
{
    namespace
    {
        bool iff2Read = false;
        bool interruptJustEnabled = false;
        bool halted = false;

        const uint8_t halfcarryAdd[8] = { 0, FLAG_H, FLAG_H, FLAG_H, 0, 0, 0, FLAG_H };
        const uint8_t halfcarrySub[8] = { 0, 0, FLAG_H, 0, FLAG_H, 0, FLAG_H, FLAG_H };
        const uint8_t overflowAdd[8] = { 0, 0, 0, FLAG_V, FLAG_V, 0, 0, 0 };
        const uint8_t overflowSub[8] = { 0, FLAG_V, 0, 0, 0, 0, FLAG_V, 0 };

        uint8_t* sz53 = nullptr;
        uint8_t* sz53p = nullptr;
        uint8_t* parity = nullptr;

        inline uint8_t memoryRead(uint16_t addr)
        {
            CONTEND(addr, 3);
            return memory::read(addr);
        }

        inline void memoryWrite(uint16_t addr, uint8_t data)
        {
            CONTEND(addr, 3);
            memory::write(addr, data);
        }

        bool retryOpCode;
        uint8_t lastQ;
        uint8_t opCode;
        uint8_t opCodeCB;
        uint8_t opCodeDD;
        uint8_t opCodeFD;
        uint8_t opCodeED;
        uint8_t opCodeXXCB;

        void(*opCodes[])();
        void(*opCodesCB[])();
        void(*opCodesDD[])();
        void(*opCodesED[])();
        void(*opCodesFD[])();
        void(*opCodesXXCB[])();

        void _NOP_() /* NOP          */ { };
        void _0x01() /* LD BC,nnnn   */ { C = memoryRead(PC++); B = memoryRead(PC++); };
        void _0x02() /* LD (BC),A    */ { MPL = BC + 1; MPH = A; memoryWrite(BC, A); };
        void _0x03() /* INC BC       */ { REP_2(CONTEND(IR, 1)); ++BC; };
        void _0x04() /* INC B        */ { INC(B); };
        void _0x05() /* DEC B        */ { DEC(B); };
        void _0x06() /* LD B,nn      */ { B = memoryRead(PC++); };
        void _0x07() /* RLCA         */ { A = (A << 1) | (A >> 7); F = (F & (FLAG_P | FLAG_Z | FLAG_S)) | (A & (FLAG_C | FLAG_3 | FLAG_5)); Q = F; };
        void _0x08() /* EX AF,AF'    */ { std::swap(AF, AF_); };
        void _0x09() /* ADD HL,BC    */ { REP_7(CONTEND_READ_NO_MREQ(IR, 1)); ADDW(HL, BC); };
        void _0x0a() /* LD A,(BC)    */ { MP = BC + 1; A = memoryRead(BC); };
        void _0x0b() /* DEC BC       */ { REP_2(CONTEND_READ_NO_MREQ(IR, 1)); --BC; };
        void _0x0c() /* INC C        */ { INC(C); };
        void _0x0d() /* DEC C        */ { DEC(C); };
        void _0x0e() /* LD C,nn      */ { C = memoryRead(PC++); };
        void _0x0f() /* RRCA         */ { F = (F & (FLAG_P | FLAG_Z | FLAG_S)) | (A & FLAG_C); A = (A >> 1) | (A << 7); F |= (A & (FLAG_3 | FLAG_5)); Q = F; };
        void _0x10() /* DJNZ offset  */ { CONTEND_READ_NO_MREQ(IR, 1); --B; if (B) { JR(); } else { CONTEND(PC, 3); ++PC; } };
        void _0x11() /* LD DE,nnnn   */ { E = memoryRead(PC++); D = memoryRead(PC++); };
        void _0x12() /* LD (DE),A    */ { MPL = DE + 1; MPH = A; memoryWrite(DE, A); };
        void _0x13() /* INC DE       */ { REP_2(CONTEND_READ_NO_MREQ(IR, 1)); ++DE; };
        void _0x14() /* INC D        */ { INC(D); };
        void _0x15() /* DEC D        */ { DEC(D); };
        void _0x16() /* LD D,nn      */ { D = memoryRead(PC++); };
        void _0x17() /* RLA          */ { uint8_t v = A; A = (A << 1) | (F & FLAG_C); F = (F & (FLAG_P | FLAG_Z | FLAG_S)) | (A & (FLAG_3 | FLAG_5)) | (v >> 7); Q = F; };
        void _0x18() /* JR offset    */ JR();
        void _0x19() /* ADD HL,DE    */ { REP_7(CONTEND_READ_NO_MREQ(IR, 1)); ADDW(HL, DE); };
        void _0x1a() /* LD A,(DE)    */ { MP = DE + 1; A = memoryRead(DE); };
        void _0x1b() /* DEC DE       */ { REP_2(CONTEND_READ_NO_MREQ(IR, 1)); --DE; };
        void _0x1c() /* INC E        */ { INC(E); };
        void _0x1d() /* DEC E        */ { DEC(E); };
        void _0x1e() /* LD E,nn      */ { E = memoryRead(PC++); };
        void _0x1f() /* RRA          */ { uint8_t v = A; A = (A >> 1) | (F << 7); F = (F & (FLAG_P | FLAG_Z | FLAG_S)) | (A & (FLAG_3 | FLAG_5)) | (v & FLAG_C); Q = F; };
        void _0x20() /* JR NZ,offset */ { if (!(F & FLAG_Z)) { JR(); } else { CONTEND(PC, 3); ++PC; } };
        void _0x21() /* LD HL,nnnn   */ { L = memoryRead(PC++); H = memoryRead(PC++); };
        void _0x22() /* LD (nnnn),HL */ LDW_NNRR(L, H);
        void _0x23() /* INC HL       */ { REP_2(CONTEND_READ_NO_MREQ(IR, 1)); ++HL; };
        void _0x24() /* INC H        */ { INC(H); };
        void _0x25() /* DEC H        */ { DEC(H); };
        void _0x26() /* LD H,nn      */ { H = memoryRead(PC++); };
        void _0x27() /* DAA          */ { uint8_t a = 0, c = (F & FLAG_C); if ((F & FLAG_H) || ((A & 0x0f) > 9)) a = 6; if (c || (A > 0x99)) a |= 0x60; if (A > 0x99) c = FLAG_C; if (F & FLAG_N) { SUB(a); } else { ADD(a); } F = (F & ~(FLAG_C | FLAG_P)) | c | parity[A]; Q = F; };
        void _0x28() /* JR Z,offset  */ { if (F & FLAG_Z) { JR(); } else { CONTEND(PC, 3); ++PC; } };
        void _0x29() /* ADD HL,HL    */ { REP_7(CONTEND_READ_NO_MREQ(IR, 1)); ADDW(HL, HL); };
        void _0x2a() /* LD HL,(nnnn) */ LDW_RRNN(L, H);
        void _0x2b() /* DEC HL       */ { REP_2(CONTEND_READ_NO_MREQ(IR, 1)); --HL; };
        void _0x2c() /* INC L        */ { INC(L); };
        void _0x2d() /* DEC L        */ { DEC(L); };
        void _0x2e() /* LD L,nn      */ { L = memoryRead(PC++); };
        void _0x2f() /* CPL          */ { A ^= 0xff; F = (F & (FLAG_C | FLAG_P | FLAG_Z | FLAG_S)) | (A & (FLAG_3 | FLAG_5)) | (FLAG_N | FLAG_H); Q = F; };
        void _0x30() /* JR NC,offset */ { if (!(F & FLAG_C)) { JR(); } else { CONTEND(PC,3); ++PC; } };
        void _0x31() /* LD SP,nnnn   */ { SPL = memoryRead(PC++); SPH = memoryRead(PC++); };
        void _0x32() /* LD (nnnn),A  */ { uint16_t v = memoryRead(PC++); v |= memoryRead(PC++) << 8; MPL = v + 1; MPH = A; memoryWrite(v, A); };
        void _0x33() /* INC SP       */ { REP_2(CONTEND_READ_NO_MREQ(IR, 1)); ++SP; };
        void _0x34() /* INC (HL)     */ { uint8_t v = memoryRead(HL); CONTEND_READ_NO_MREQ(HL, 1); INC(v); memoryWrite(HL, v); };
        void _0x35() /* DEC (HL)     */ { uint8_t v = memoryRead(HL); CONTEND_READ_NO_MREQ(HL, 1); DEC(v); memoryWrite(HL, v); };
        void _0x36() /* LD (HL),nn   */ { memoryWrite(HL, memoryRead(PC++)); };
        void _0x37() /* SCF          */ { F = (F & (FLAG_P | FLAG_Z | FLAG_S)) | ((IS_CMOS ? A : ((lastQ ^ F) | A)) & (FLAG_3 | FLAG_5)) | FLAG_C; Q = F; };
        void _0x38() /* JR C,offset  */ { if (F & FLAG_C) { JR(); } else { CONTEND(PC, 3); ++PC; } };
        void _0x39() /* ADD HL,SP    */ { REP_7(CONTEND_READ_NO_MREQ(IR, 1)); ADDW(HL, SP); };
        void _0x3a() /* LD A,(nnnn)  */ { MPL = memoryRead(PC++); MPH = memoryRead(PC++); A = memoryRead(MP++); };
        void _0x3b() /* DEC SP       */ { REP_2(CONTEND_READ_NO_MREQ(IR, 1)); --SP; };
        void _0x3c() /* INC A        */ { INC(A); };
        void _0x3d() /* DEC A        */ { DEC(A); };
        void _0x3e() /* LD A,nn      */ { A = memoryRead(PC++); };
        void _0x3f() /* CCF          */ { F = (F & (FLAG_P | FLAG_Z | FLAG_S)) | ((F & FLAG_C) ? FLAG_H : FLAG_C) | ((IS_CMOS ? A : ((lastQ ^ F) | A)) & (FLAG_3 | FLAG_5)); Q = F; };
        void _0x41() /* LD B,C       */ { B = C; };
        void _0x42() /* LD B,D       */ { B = D; };
        void _0x43() /* LD B,E       */ { B = E; };
        void _0x44() /* LD B,H       */ { B = H; };
        void _0x45() /* LD B,L       */ { B = L; };
        void _0x46() /* LD B,(HL)    */ { B = memoryRead(HL); };
        void _0x47() /* LD B,A       */ { B = A; };
        void _0x48() /* LD C,B       */ { C = B; };
        void _0x4a() /* LD C,D       */ { C = D; };
        void _0x4b() /* LD C,E       */ { C = E; };
        void _0x4c() /* LD C,H       */ { C = H; };
        void _0x4d() /* LD C,L       */ { C = L; };
        void _0x4e() /* LD C,(HL)    */ { C = memoryRead(HL); };
        void _0x4f() /* LD C,A       */ { C = A; };
        void _0x50() /* LD D,B       */ { D = B; };
        void _0x51() /* LD D,C       */ { D = C; };
        void _0x53() /* LD D,E       */ { D = E; };
        void _0x54() /* LD D,H       */ { D = H; };
        void _0x55() /* LD D,L       */ { D = L; };
        void _0x56() /* LD D,(HL)    */ { D = memoryRead(HL); };
        void _0x57() /* LD D,A       */ { D = A; };
        void _0x58() /* LD E,B       */ { E = B; };
        void _0x59() /* LD E,C       */ { E = C; };
        void _0x5a() /* LD E,D       */ { E = D; };
        void _0x5c() /* LD E,H       */ { E = H; };
        void _0x5d() /* LD E,L       */ { E = L; };
        void _0x5e() /* LD E,(HL)    */ { E = memoryRead(HL); };
        void _0x5f() /* LD E,A       */ { E = A; };
        void _0x60() /* LD H,B       */ { H = B; };
        void _0x61() /* LD H,C       */ { H = C; };
        void _0x62() /* LD H,D       */ { H = D; };
        void _0x63() /* LD H,E       */ { H = E; };
        void _0x65() /* LD H,L       */ { H = L; };
        void _0x66() /* LD H,(HL)    */ { H = memoryRead(HL); };
        void _0x67() /* LD H,A       */ { H = A; };
        void _0x68() /* LD L,B       */ { L = B; };
        void _0x69() /* LD L,C       */ { L = C; };
        void _0x6a() /* LD L,D       */ { L = D; };
        void _0x6b() /* LD L,E       */ { L = E; };
        void _0x6c() /* LD L,H       */ { L = H; };
        void _0x6e() /* LD L,(HL)    */ { L = memoryRead(HL); };
        void _0x6f() /* LD L,A       */ { L = A; };
        void _0x70() /* LD (HL),B    */ { memoryWrite(HL, B); };
        void _0x71() /* LD (HL),C    */ { memoryWrite(HL, C); };
        void _0x72() /* LD (HL),D    */ { memoryWrite(HL, D); };
        void _0x73() /* LD (HL),E    */ { memoryWrite(HL, E); };
        void _0x74() /* LD (HL),H    */ { memoryWrite(HL, H); };
        void _0x75() /* LD (HL),L    */ { memoryWrite(HL, L); };
        void _0x76() /* HALT         */ { halted = true; --PC; };
        void _0x77() /* LD (HL),A    */ { memoryWrite(HL, A); };
        void _0x78() /* LD A,B       */ { A = B; };
        void _0x79() /* LD A,C       */ { A = C; };
        void _0x7a() /* LD A,D       */ { A = D; };
        void _0x7b() /* LD A,E       */ { A = E; };
        void _0x7c() /* LD A,H       */ { A = H; };
        void _0x7d() /* LD A,L       */ { A = L; };
        void _0x7e() /* LD A,(HL)    */ { A = memoryRead(HL); };
        void _0x80() /* ADD A,B      */ ADD(B);
        void _0x81() /* ADD A,C      */ ADD(C);
        void _0x82() /* ADD A,D      */ ADD(D);
        void _0x83() /* ADD A,E      */ ADD(E);
        void _0x84() /* ADD A,H      */ ADD(H);
        void _0x85() /* ADD A,L      */ ADD(L);
        void _0x86() /* ADD A,(HL)   */ { uint8_t v = memoryRead(HL); ADD(v); };
        void _0x87() /* ADD A,A      */ ADD(A);
        void _0x88() /* ADC A,B      */ ADC(B);
        void _0x89() /* ADC A,C      */ ADC(C);
        void _0x8a() /* ADC A,D      */ ADC(D);
        void _0x8b() /* ADC A,E      */ ADC(E);
        void _0x8c() /* ADC A,H      */ ADC(H);
        void _0x8d() /* ADC A,L      */ ADC(L);
        void _0x8e() /* ADC A,(HL)   */ { uint8_t v = memoryRead(HL); ADC(v); };
        void _0x8f() /* ADC A,A      */ ADC(A);
        void _0x90() /* SUB A,B      */ SUB(B);
        void _0x91() /* SUB A,C      */ SUB(C);
        void _0x92() /* SUB A,D      */ SUB(D);
        void _0x93() /* SUB A,E      */ SUB(E);
        void _0x94() /* SUB A,H      */ SUB(H);
        void _0x95() /* SUB A,L      */ SUB(L);
        void _0x96() /* SUB A,(HL)   */ { uint8_t v = memoryRead(HL); SUB(v); };
        void _0x97() /* SUB A,A      */ SUB(A);
        void _0x98() /* SBC A,B      */ SBC(B);
        void _0x99() /* SBC A,C      */ SBC(C);
        void _0x9a() /* SBC A,D      */ SBC(D);
        void _0x9b() /* SBC A,E      */ SBC(E);
        void _0x9c() /* SBC A,H      */ SBC(H);
        void _0x9d() /* SBC A,L      */ SBC(L);
        void _0x9e() /* SBC A,(HL)   */ { uint8_t v = memoryRead(HL); SBC(v); };
        void _0x9f() /* SBC A,A      */ SBC(A);
        void _0xa0() /* AND A,B      */ { AND(B); };
        void _0xa1() /* AND A,C      */ { AND(C); };
        void _0xa2() /* AND A,D      */ { AND(D); };
        void _0xa3() /* AND A,E      */ { AND(E); };
        void _0xa4() /* AND A,H      */ { AND(H); };
        void _0xa5() /* AND A,L      */ { AND(L); };
        void _0xa6() /* AND A,(HL)   */ { uint8_t v = memoryRead(HL); AND(v); };
        void _0xa7() /* AND A,A      */ { AND(A); };
        void _0xa8() /* XOR A,B      */ { XOR(B); };
        void _0xa9() /* XOR A,C      */ { XOR(C); };
        void _0xaa() /* XOR A,D      */ { XOR(D); };
        void _0xab() /* XOR A,E      */ { XOR(E); };
        void _0xac() /* XOR A,H      */ { XOR(H); };
        void _0xad() /* XOR A,L      */ { XOR(L); };
        void _0xae() /* XOR A,(HL)   */ { uint8_t v = memoryRead(HL); XOR(v); };
        void _0xaf() /* XOR A,A      */ { XOR(A); };
        void _0xb0() /* OR A,B       */ { OR(B); };
        void _0xb1() /* OR A,C       */ { OR(C); };
        void _0xb2() /* OR A,D       */ { OR(D); };
        void _0xb3() /* OR A,E       */ { OR(E); };
        void _0xb4() /* OR A,H       */ { OR(H); };
        void _0xb5() /* OR A,L       */ { OR(L); };
        void _0xb6() /* OR A,(HL)    */ { uint8_t v = memoryRead(HL); OR(v); };
        void _0xb7() /* OR A,A       */ { OR(A); };
        void _0xb8() /* CP B         */ CP(B);
        void _0xb9() /* CP C         */ CP(C);
        void _0xba() /* CP D         */ CP(D);
        void _0xbb() /* CP E         */ CP(E);
        void _0xbc() /* CP H         */ CP(H);
        void _0xbd() /* CP L         */ CP(L);
        void _0xbe() /* CP (HL)      */ { uint8_t v = memoryRead(HL); CP(v); };
        void _0xbf() /* CP A         */ CP(A);
        void _0xc0() /* RET NZ       */ { CONTEND_READ_NO_MREQ(IR, 1); if (!(F & FLAG_Z)) { RET(); } };
        void _0xc1() /* POP BC       */ { POPW(C, B); };
        void _0xc2() /* JP NZ,nnnn   */ { MPL = memoryRead(PC++); MPH = memoryRead(PC); if (!(F & FLAG_Z)) { JP(); } else { ++PC; } };
        void _0xc3() /* JP nnnn      */ { MPL = memoryRead(PC++); MPH = memoryRead(PC); JP(); };
        void _0xc4() /* CALL NZ,nnnn */ { MPL = memoryRead(PC++); MPH = memoryRead(PC); if (!(F & FLAG_Z)) { CALL(); } else { ++PC; } };
        void _0xc5() /* PUSH BC      */ { CONTEND_READ_NO_MREQ(IR, 1); PUSHW(C, B); };
        void _0xc6() /* ADD A,nn     */ { uint8_t v = memoryRead(PC++); ADD(v); };
        void _0xc7() /* RST 00       */ { CONTEND_READ_NO_MREQ(IR, 1); RST(0x00); };
        void _0xc8() /* RET Z        */ { CONTEND_READ_NO_MREQ(IR, 1); if (F & FLAG_Z) { RET(); } };
        void _0xc9() /* RET          */ { RET(); };
        void _0xca() /* JP Z,nnnn    */ { MPL = memoryRead(PC++); MPH = memoryRead(PC); if (F & FLAG_Z) { JP(); } else { ++PC; } };
        void _0xcb()                    { CONTEND(PC, 4); opCodeCB = memory::read(PC++); ++R; opCodesCB[opCodeCB](); };
        void _0xcc() /* CALL Z,nnnn  */ { MPL = memoryRead(PC++); MPH = memoryRead(PC); if (F & FLAG_Z) { CALL(); } else { ++PC; } };
        void _0xcd() /* CALL nnnn    */ { MPL = memoryRead(PC++); MPH = memoryRead(PC); CALL(); };
        void _0xce() /* ADC A,nn     */ { uint8_t v = memoryRead(PC++); ADC(v); };
        void _0xcf() /* RST 8        */ { CONTEND_READ_NO_MREQ(IR, 1); RST(0x08); };
        void _0xd0() /* RET NC       */ { CONTEND_READ_NO_MREQ(IR, 1); if (!(F & FLAG_C)) { RET(); } };
        void _0xd1() /* POP DE       */ { POPW(E, D); };
        void _0xd2() /* JP NC,nnnn   */ { MPL = memoryRead(PC++); MPH = memoryRead(PC); if (!(F & FLAG_C)) { JP(); } else { ++PC; } };
        void _0xd3() /* OUT (nn),A   */ { uint8_t v = memoryRead(PC++); uint16_t port = v | (A << 8); MPH = A; MPL = v + 1; io::write(port, A); };
        void _0xd4() /* CALL NC,nnnn */ { MPL = memoryRead(PC++); MPH = memoryRead(PC); if (!(F & FLAG_C)) { CALL(); } else { ++PC; } };
        void _0xd5() /* PUSH DE      */ { CONTEND_READ_NO_MREQ(IR, 1); PUSHW(E, D); };
        void _0xd6() /* SUB nn       */ { uint8_t v = memoryRead(PC++); SUB(v); };
        void _0xd7() /* RST 10       */ { CONTEND_READ_NO_MREQ(IR, 1); RST(0x10); };
        void _0xd8() /* RET C        */ { CONTEND_READ_NO_MREQ(IR, 1); if (F & FLAG_C) { RET(); } };
        void _0xd9() /* EXX          */ { std::swap(BC, BC_); std::swap(DE, DE_); std::swap(HL, HL_); };
        void _0xda() /* JP C,nnnn    */ { MPL = memoryRead(PC++); MPH = memoryRead(PC); if (F & FLAG_C) { JP(); } else { ++PC; } };
        void _0xdb() /* IN A,(nn)    */ { uint16_t port = memoryRead(PC++) + (A << 8); A = io::read(port); MP = port + 1; };
        void _0xdc() /* CALL C,nnnn  */ { MPL = memoryRead(PC++); MPH = memoryRead(PC); if (F & FLAG_C) { CALL(); } else { ++PC; } };
        void _0xdd()                    { CONTEND(PC, 4); opCodeDD = memory::read(PC++); ++R; opCodesDD[opCodeDD](); };
        void _0xde() /* SBC A,nn     */ { uint8_t v = memoryRead(PC++); SBC(v); };
        void _0xdf() /* RST 18       */ { CONTEND_READ_NO_MREQ(IR, 1); RST(0x18); };
        void _0xe0() /* RET PO       */ { CONTEND_READ_NO_MREQ(IR, 1); if (!(F & FLAG_P)) { RET(); } };
        void _0xe1() /* POP HL       */ { POPW(L, H); };
        void _0xe2() /* JP PO,nnnn   */ { MPL = memoryRead(PC++); MPH = memoryRead(PC); if (!(F & FLAG_P)) { JP(); } else { ++PC; } };
        void _0xe3() /* EX (SP),HL   */ { uint8_t vl = memoryRead(SP), vh = memoryRead(SP + 1); CONTEND_READ_NO_MREQ(SP + 1, 1); memoryWrite(SP + 1, H); memoryWrite(SP, L); REP_2(CONTEND_READ_NO_MREQ(SP, 1)); L = MPL = vl; H = MPH = vh; };
        void _0xe4() /* CALL PO,nnnn */ { MPL = memoryRead(PC++); MPH = memoryRead(PC); if (!(F & FLAG_P)) { CALL(); } else { ++PC; } };
        void _0xe5() /* PUSH HL      */ { CONTEND_READ_NO_MREQ(IR, 1); PUSHW(L, H); };
        void _0xe6() /* AND nn       */ { AND(memoryRead(PC++)); };
        void _0xe7() /* RST 20       */ { CONTEND_READ_NO_MREQ(IR, 1); RST(0x20); };
        void _0xe8() /* RET PE       */ { CONTEND_READ_NO_MREQ(IR, 1); if (F & FLAG_P) { RET(); } };
        void _0xe9() /* JP HL        */ { PC = HL; };
        void _0xea() /* JP PE,nnnn   */ { MPL = memoryRead(PC++); MPH = memoryRead(PC); if (F & FLAG_P) { JP(); } else { ++PC; } };
        void _0xeb() /* EX DE,HL     */ { std::swap(DE, HL);};
        void _0xec() /* CALL PE,nnnn */ { MPL = memoryRead(PC++); MPH = memoryRead(PC); if (F & FLAG_P) { CALL(); } else { ++PC; } };
        void _0xed()                    { CONTEND(PC, 4); opCodeED = memory::read(PC++); ++R; opCodesED[opCodeED](); };
        void _0xee() /* XOR A,nn     */ { XOR(memoryRead(PC++)); };
        void _0xef() /* RST 28       */ { CONTEND_READ_NO_MREQ(IR, 1); RST(0x28); };
        void _0xf0() /* RET P        */ { CONTEND_READ_NO_MREQ(IR, 1); if (!(F & FLAG_S)) { RET(); } };
        void _0xf1() /* POP AF       */ { POPW(F, A); };
        void _0xf2() /* JP P,nnnn    */ { MPL = memoryRead(PC++); MPH = memoryRead(PC); if (!(F & FLAG_S)) { JP(); } else { ++PC; } };
        void _0xf3() /* DI           */ { IFF1 = false; IFF2 = false; };
        void _0xf4() /* CALL P,nnnn  */ { MPL = memoryRead(PC++); MPH = memoryRead(PC); if (!(F & FLAG_S)) { CALL(); } else { ++PC; } };
        void _0xf5() /* PUSH AF      */ { CONTEND_READ_NO_MREQ(IR, 1); PUSHW(F, A); };
        void _0xf6() /* OR nn        */ { OR(memoryRead(PC++)); };
        void _0xf7() /* RST 30       */ { CONTEND_READ_NO_MREQ(IR, 1); RST(0x30); };
        void _0xf8() /* RET M        */ { CONTEND_READ_NO_MREQ(IR, 1); if (F & FLAG_S) { RET(); } };
        void _0xf9() /* LD SP,HL     */ { CONTEND_READ_NO_MREQ(IR, 1); CONTEND_READ_NO_MREQ(IR, 1); SP = HL; };
        void _0xfa() /* JP M,nnnn    */ { MPL = memoryRead(PC++); MPH = memoryRead(PC); if (F & FLAG_S) { JP(); } else { ++PC; } };
        void _0xfb() /* EI           */ { IFF1 = true; IFF2 = true; interruptJustEnabled = true; };
        void _0xfc() /* CALL M,nnnn  */ { MPL = memoryRead(PC++); MPH = memoryRead(PC); if (F & FLAG_S) { CALL(); } else { ++PC; } };
        void _0xfd()                    { CONTEND(PC, 4); opCodeFD = memory::read(PC++); ++R; opCodesFD[opCodeFD](); };
        void _0xfe() /* CP nn        */ { uint8_t v = memoryRead(PC++); CP(v); };
        void _0xff() /* RST 38       */ { CONTEND_READ_NO_MREQ(IR, 1); RST(0x38); };

        void _0xcb00() /* RLC B      */ { RLC(B); };
        void _0xcb01() /* RLC C      */ { RLC(C); };
        void _0xcb02() /* RLC D      */ { RLC(D); };
        void _0xcb03() /* RLC E      */ { RLC(E); };
        void _0xcb04() /* RLC H      */ { RLC(H); };
        void _0xcb05() /* RLC L      */ { RLC(L); };
        void _0xcb06() /* RLC (HL)   */ { uint8_t t = memoryRead(HL); CONTEND_READ_NO_MREQ(HL, 1); RLC(t); memoryWrite(HL, t); };
        void _0xcb07() /* RLC A      */ { RLC(A); };
        void _0xcb08() /* RRC B      */ { RRC(B); };
        void _0xcb09() /* RRC C      */ { RRC(C); };
        void _0xcb0a() /* RRC D      */ { RRC(D); };
        void _0xcb0b() /* RRC E      */ { RRC(E); };
        void _0xcb0c() /* RRC H      */ { RRC(H); };
        void _0xcb0d() /* RRC L      */ { RRC(L); };
        void _0xcb0e() /* RRC (HL)   */ { uint8_t t = memoryRead(HL); CONTEND_READ_NO_MREQ(HL, 1); RRC(t); memoryWrite(HL, t); };
        void _0xcb0f() /* RRC A      */ { RRC(A); };
        void _0xcb10() /* RL B       */ RL(B);
        void _0xcb11() /* RL C       */ RL(C);
        void _0xcb12() /* RL D       */ RL(D);
        void _0xcb13() /* RL E       */ RL(E);
        void _0xcb14() /* RL H       */ RL(H);
        void _0xcb15() /* RL L       */ RL(L);
        void _0xcb16() /* RL (HL)    */ { uint8_t t = memoryRead(HL); CONTEND_READ_NO_MREQ(HL, 1); RL(t); memoryWrite(HL, t); };
        void _0xcb17() /* RL A       */ RL(A);
        void _0xcb18() /* RR B       */ RR(B);
        void _0xcb19() /* RR C       */ RR(C);
        void _0xcb1a() /* RR D       */ RR(D);
        void _0xcb1b() /* RR E       */ RR(E);
        void _0xcb1c() /* RR H       */ RR(H);
        void _0xcb1d() /* RR L       */ RR(L);
        void _0xcb1e() /* RR (HL)    */ { uint8_t t = memoryRead(HL); CONTEND_READ_NO_MREQ(HL, 1); RR(t); memoryWrite(HL, t); };
        void _0xcb1f() /* RR A       */ RR(A);
        void _0xcb20() /* SLA B      */ { SLA(B); };
        void _0xcb21() /* SLA C      */ { SLA(C); };
        void _0xcb22() /* SLA D      */ { SLA(D); };
        void _0xcb23() /* SLA E      */ { SLA(E); };
        void _0xcb24() /* SLA H      */ { SLA(H); };
        void _0xcb25() /* SLA L      */ { SLA(L); };
        void _0xcb26() /* SLA (HL)   */ { uint8_t t = memoryRead(HL); CONTEND_READ_NO_MREQ(HL, 1); SLA(t); memoryWrite(HL, t); };
        void _0xcb27() /* SLA A      */ { SLA(A); };
        void _0xcb28() /* SRA B      */ { SRA(B); };
        void _0xcb29() /* SRA C      */ { SRA(C); };
        void _0xcb2a() /* SRA D      */ { SRA(D); };
        void _0xcb2b() /* SRA E      */ { SRA(E); };
        void _0xcb2c() /* SRA H      */ { SRA(H); };
        void _0xcb2d() /* SRA L      */ { SRA(L); };
        void _0xcb2e() /* SRA (HL)   */ { uint8_t t = memoryRead(HL); CONTEND_READ_NO_MREQ(HL, 1); SRA(t); memoryWrite(HL, t); };
        void _0xcb2f() /* SRA A      */ { SRA(A); };
        void _0xcb30() /* SLL B      */ { SLL(B); };
        void _0xcb31() /* SLL C      */ { SLL(C); };
        void _0xcb32() /* SLL D      */ { SLL(D); };
        void _0xcb33() /* SLL E      */ { SLL(E); };
        void _0xcb34() /* SLL H      */ { SLL(H); };
        void _0xcb35() /* SLL L      */ { SLL(L); };
        void _0xcb36() /* SLL (HL)   */ { uint8_t t = memoryRead(HL); CONTEND_READ_NO_MREQ(HL, 1); SLL(t); memoryWrite(HL, t); };
        void _0xcb37() /* SLL A      */ { SLL(A); };
        void _0xcb38() /* SRL B      */ { SRL(B); };
        void _0xcb39() /* SRL C      */ { SRL(C); };
        void _0xcb3a() /* SRL D      */ { SRL(D); };
        void _0xcb3b() /* SRL E      */ { SRL(E); };
        void _0xcb3c() /* SRL H      */ { SRL(H); };
        void _0xcb3d() /* SRL L      */ { SRL(L); };
        void _0xcb3e() /* SRL (HL)   */ { uint8_t t = memoryRead(HL); CONTEND_READ_NO_MREQ(HL, 1); SRL(t); memoryWrite(HL, t); };
        void _0xcb3f() /* SRL A      */ { SRL(A); };
        void _0xcb40() /* BIT 0,B    */ { BIT(0, B); };
        void _0xcb41() /* BIT 0,C    */ { BIT(0, C); };
        void _0xcb42() /* BIT 0,D    */ { BIT(0, D); };
        void _0xcb43() /* BIT 0,E    */ { BIT(0, E); };
        void _0xcb44() /* BIT 0,H    */ { BIT(0, H); };
        void _0xcb45() /* BIT 0,L    */ { BIT(0, L); };
        void _0xcb46() /* BIT 0,(HL) */ { uint8_t t = memoryRead(HL); CONTEND_READ_NO_MREQ(HL, 1); BIT_MP(0, t); };
        void _0xcb47() /* BIT 0,A    */ { BIT(0, A); };
        void _0xcb48() /* BIT 1,B    */ { BIT(1, B); };
        void _0xcb49() /* BIT 1,C    */ { BIT(1, C); };
        void _0xcb4a() /* BIT 1,D    */ { BIT(1, D); };
        void _0xcb4b() /* BIT 1,E    */ { BIT(1, E); };
        void _0xcb4c() /* BIT 1,H    */ { BIT(1, H); };
        void _0xcb4d() /* BIT 1,L    */ { BIT(1, L); };
        void _0xcb4e() /* BIT 1,(HL) */ { uint8_t t = memoryRead(HL); CONTEND_READ_NO_MREQ(HL, 1); BIT_MP(1, t); };
        void _0xcb4f() /* BIT 1,A    */ { BIT(1, A); };
        void _0xcb50() /* BIT 2,B    */ { BIT(2, B); };
        void _0xcb51() /* BIT 2,C    */ { BIT(2, C); };
        void _0xcb52() /* BIT 2,D    */ { BIT(2, D); };
        void _0xcb53() /* BIT 2,E    */ { BIT(2, E); };
        void _0xcb54() /* BIT 2,H    */ { BIT(2, H); };
        void _0xcb55() /* BIT 2,L    */ { BIT(2, L); };
        void _0xcb56() /* BIT 2,(HL) */ { uint8_t t = memoryRead(HL); CONTEND_READ_NO_MREQ(HL, 1); BIT_MP(2, t); };
        void _0xcb57() /* BIT 2,A    */ { BIT(2, A); };
        void _0xcb58() /* BIT 3,B    */ { BIT(3, B); };
        void _0xcb59() /* BIT 3,C    */ { BIT(3, C); };
        void _0xcb5a() /* BIT 3,D    */ { BIT(3, D); };
        void _0xcb5b() /* BIT 3,E    */ { BIT(3, E); };
        void _0xcb5c() /* BIT 3,H    */ { BIT(3, H); };
        void _0xcb5d() /* BIT 3,L    */ { BIT(3, L); };
        void _0xcb5e() /* BIT 3,(HL) */ { uint8_t t = memoryRead(HL); CONTEND_READ_NO_MREQ(HL, 1); BIT_MP(3, t); };
        void _0xcb5f() /* BIT 3,A    */ { BIT(3, A); };
        void _0xcb60() /* BIT 4,B    */ { BIT(4, B); };
        void _0xcb61() /* BIT 4,C    */ { BIT(4, C); };
        void _0xcb62() /* BIT 4,D    */ { BIT(4, D); };
        void _0xcb63() /* BIT 4,E    */ { BIT(4, E); };
        void _0xcb64() /* BIT 4,H    */ { BIT(4, H); };
        void _0xcb65() /* BIT 4,L    */ { BIT(4, L); };
        void _0xcb66() /* BIT 4,(HL) */ { uint8_t t = memoryRead(HL); CONTEND_READ_NO_MREQ(HL, 1); BIT_MP(4, t); };
        void _0xcb67() /* BIT 4,A    */ { BIT(4, A); };
        void _0xcb68() /* BIT 5,B    */ { BIT(5, B); };
        void _0xcb69() /* BIT 5,C    */ { BIT(5, C); };
        void _0xcb6a() /* BIT 5,D    */ { BIT(5, D); };
        void _0xcb6b() /* BIT 5,E    */ { BIT(5, E); };
        void _0xcb6c() /* BIT 5,H    */ { BIT(5, H); };
        void _0xcb6d() /* BIT 5,L    */ { BIT(5, L); };
        void _0xcb6e() /* BIT 5,(HL) */ { uint8_t t = memoryRead(HL); CONTEND_READ_NO_MREQ(HL, 1); BIT_MP(5, t); };
        void _0xcb6f() /* BIT 5,A    */ { BIT(5, A); };
        void _0xcb70() /* BIT 6,B    */ { BIT(6, B); };
        void _0xcb71() /* BIT 6,C    */ { BIT(6, C); };
        void _0xcb72() /* BIT 6,D    */ { BIT(6, D); };
        void _0xcb73() /* BIT 6,E    */ { BIT(6, E); };
        void _0xcb74() /* BIT 6,H    */ { BIT(6, H); };
        void _0xcb75() /* BIT 6,L    */ { BIT(6, L); };
        void _0xcb76() /* BIT 6,(HL) */ { uint8_t t = memoryRead(HL); CONTEND_READ_NO_MREQ(HL, 1); BIT_MP(6, t); };
        void _0xcb77() /* BIT 6,A    */ { BIT(6, A); };
        void _0xcb78() /* BIT 7,B    */ { BIT(7, B); };
        void _0xcb79() /* BIT 7,C    */ { BIT(7, C); };
        void _0xcb7a() /* BIT 7,D    */ { BIT(7, D); };
        void _0xcb7b() /* BIT 7,E    */ { BIT(7, E); };
        void _0xcb7c() /* BIT 7,H    */ { BIT(7, H); };
        void _0xcb7d() /* BIT 7,L    */ { BIT(7, L); };
        void _0xcb7e() /* BIT 7,(HL) */ { uint8_t t = memoryRead(HL); CONTEND_READ_NO_MREQ(HL, 1); BIT_MP(7, t); };
        void _0xcb7f() /* BIT 7,A    */ { BIT(7, A); };
        void _0xcb80() /* RES 0,B    */ { B &= 0xfe; };
        void _0xcb81() /* RES 0,C    */ { C &= 0xfe; };
        void _0xcb82() /* RES 0,D    */ { D &= 0xfe; };
        void _0xcb83() /* RES 0,E    */ { E &= 0xfe; };
        void _0xcb84() /* RES 0,H    */ { H &= 0xfe; };
        void _0xcb85() /* RES 0,L    */ { L &= 0xfe; };
        void _0xcb86() /* RES 0,(HL) */ { uint8_t t = memoryRead(HL); CONTEND_READ_NO_MREQ(HL, 1); memoryWrite(HL, t & 0xfe); };
        void _0xcb87() /* RES 0,A    */ { A &= 0xfe; };
        void _0xcb88() /* RES 1,B    */ { B &= 0xfd; };
        void _0xcb89() /* RES 1,C    */ { C &= 0xfd; };
        void _0xcb8a() /* RES 1,D    */ { D &= 0xfd; };
        void _0xcb8b() /* RES 1,E    */ { E &= 0xfd; };
        void _0xcb8c() /* RES 1,H    */ { H &= 0xfd; };
        void _0xcb8d() /* RES 1,L    */ { L &= 0xfd; };
        void _0xcb8e() /* RES 1,(HL) */ { uint8_t t = memoryRead(HL); CONTEND_READ_NO_MREQ(HL, 1); memoryWrite(HL, t & 0xfd); };
        void _0xcb8f() /* RES 1,A    */ { A &= 0xfd; };
        void _0xcb90() /* RES 2,B    */ { B &= 0xfb; };
        void _0xcb91() /* RES 2,C    */ { C &= 0xfb; };
        void _0xcb92() /* RES 2,D    */ { D &= 0xfb; };
        void _0xcb93() /* RES 2,E    */ { E &= 0xfb; };
        void _0xcb94() /* RES 2,H    */ { H &= 0xfb; };
        void _0xcb95() /* RES 2,L    */ { L &= 0xfb; };
        void _0xcb96() /* RES 2,(HL) */ { uint8_t t = memoryRead(HL); CONTEND_READ_NO_MREQ(HL, 1); memoryWrite(HL, t & 0xfb); };
        void _0xcb97() /* RES 2,A    */ { A &= 0xfb; };
        void _0xcb98() /* RES 3,B    */ { B &= 0xf7; };
        void _0xcb99() /* RES 3,C    */ { C &= 0xf7; };
        void _0xcb9a() /* RES 3,D    */ { D &= 0xf7; };
        void _0xcb9b() /* RES 3,E    */ { E &= 0xf7; };
        void _0xcb9c() /* RES 3,H    */ { H &= 0xf7; };
        void _0xcb9d() /* RES 3,L    */ { L &= 0xf7; };
        void _0xcb9e() /* RES 3,(HL) */ { uint8_t t = memoryRead(HL); CONTEND_READ_NO_MREQ(HL, 1); memoryWrite(HL, t & 0xf7); };
        void _0xcb9f() /* RES 3,A    */ { A &= 0xf7; };
        void _0xcba0() /* RES 4,B    */ { B &= 0xef; };
        void _0xcba1() /* RES 4,C    */ { C &= 0xef; };
        void _0xcba2() /* RES 4,D    */ { D &= 0xef; };
        void _0xcba3() /* RES 4,E    */ { E &= 0xef; };
        void _0xcba4() /* RES 4,H    */ { H &= 0xef; };
        void _0xcba5() /* RES 4,L    */ { L &= 0xef; };
        void _0xcba6() /* RES 4,(HL) */ { uint8_t t = memoryRead(HL); CONTEND_READ_NO_MREQ(HL, 1); memoryWrite(HL, t & 0xef); };
        void _0xcba7() /* RES 4,A    */ { A &= 0xef; };
        void _0xcba8() /* RES 5,B    */ { B &= 0xdf; };
        void _0xcba9() /* RES 5,C    */ { C &= 0xdf; };
        void _0xcbaa() /* RES 5,D    */ { D &= 0xdf; };
        void _0xcbab() /* RES 5,E    */ { E &= 0xdf; };
        void _0xcbac() /* RES 5,H    */ { H &= 0xdf; };
        void _0xcbad() /* RES 5,L    */ { L &= 0xdf; };
        void _0xcbae() /* RES 5,(HL) */ { uint8_t t = memoryRead(HL); CONTEND_READ_NO_MREQ(HL, 1); memoryWrite(HL, t & 0xdf); };
        void _0xcbaf() /* RES 5,A    */ { A &= 0xdf; };
        void _0xcbb0() /* RES 6,B    */ { B &= 0xbf; };
        void _0xcbb1() /* RES 6,C    */ { C &= 0xbf; };
        void _0xcbb2() /* RES 6,D    */ { D &= 0xbf; };
        void _0xcbb3() /* RES 6,E    */ { E &= 0xbf; };
        void _0xcbb4() /* RES 6,H    */ { H &= 0xbf; };
        void _0xcbb5() /* RES 6,L    */ { L &= 0xbf; };
        void _0xcbb6() /* RES 6,(HL) */ { uint8_t t = memoryRead(HL); CONTEND_READ_NO_MREQ(HL, 1); memoryWrite(HL, t & 0xbf); };
        void _0xcbb7() /* RES 6,A    */ { A &= 0xbf; };
        void _0xcbb8() /* RES 7,B    */ { B &= 0x7f; };
        void _0xcbb9() /* RES 7,C    */ { C &= 0x7f; };
        void _0xcbba() /* RES 7,D    */ { D &= 0x7f; };
        void _0xcbbb() /* RES 7,E    */ { E &= 0x7f; };
        void _0xcbbc() /* RES 7,H    */ { H &= 0x7f; };
        void _0xcbbd() /* RES 7,L    */ { L &= 0x7f; };
        void _0xcbbe() /* RES 7,(HL) */ { uint8_t t = memoryRead(HL); CONTEND_READ_NO_MREQ(HL, 1); memoryWrite(HL, t & 0x7f); };
        void _0xcbbf() /* RES 7,A    */ { A &= 0x7f; };
        void _0xcbc0() /* SET 0,B    */ { B |= 0x01; };
        void _0xcbc1() /* SET 0,C    */ { C |= 0x01; };
        void _0xcbc2() /* SET 0,D    */ { D |= 0x01; };
        void _0xcbc3() /* SET 0,E    */ { E |= 0x01; };
        void _0xcbc4() /* SET 0,H    */ { H |= 0x01; };
        void _0xcbc5() /* SET 0,L    */ { L |= 0x01; };
        void _0xcbc6() /* SET 0,(HL) */ { uint8_t t = memoryRead(HL); CONTEND_READ_NO_MREQ(HL, 1); memoryWrite(HL, t | 0x01); };
        void _0xcbc7() /* SET 0,A    */ { A |= 0x01; };
        void _0xcbc8() /* SET 1,B    */ { B |= 0x02; };
        void _0xcbc9() /* SET 1,C    */ { C |= 0x02; };
        void _0xcbca() /* SET 1,D    */ { D |= 0x02; };
        void _0xcbcb() /* SET 1,E    */ { E |= 0x02; };
        void _0xcbcc() /* SET 1,H    */ { H |= 0x02; };
        void _0xcbcd() /* SET 1,L    */ { L |= 0x02; };
        void _0xcbce() /* SET 1,(HL) */ { uint8_t t = memoryRead(HL); CONTEND_READ_NO_MREQ(HL, 1); memoryWrite(HL, t | 0x02); };
        void _0xcbcf() /* SET 1,A    */ { A |= 0x02; };
        void _0xcbd0() /* SET 2,B    */ { B |= 0x04; };
        void _0xcbd1() /* SET 2,C    */ { C |= 0x04; };
        void _0xcbd2() /* SET 2,D    */ { D |= 0x04; };
        void _0xcbd3() /* SET 2,E    */ { E |= 0x04; };
        void _0xcbd4() /* SET 2,H    */ { H |= 0x04; };
        void _0xcbd5() /* SET 2,L    */ { L |= 0x04; };
        void _0xcbd6() /* SET 2,(HL) */ { uint8_t t = memoryRead(HL); CONTEND_READ_NO_MREQ(HL, 1); memoryWrite(HL, t | 0x04); };
        void _0xcbd7() /* SET 2,A    */ { A |= 0x04; };
        void _0xcbd8() /* SET 3,B    */ { B |= 0x08; };
        void _0xcbd9() /* SET 3,C    */ { C |= 0x08; };
        void _0xcbda() /* SET 3,D    */ { D |= 0x08; };
        void _0xcbdb() /* SET 3,E    */ { E |= 0x08; };
        void _0xcbdc() /* SET 3,H    */ { H |= 0x08; };
        void _0xcbdd() /* SET 3,L    */ { L |= 0x08; };
        void _0xcbde() /* SET 3,(HL) */ { uint8_t t = memoryRead(HL); CONTEND_READ_NO_MREQ(HL, 1); memoryWrite(HL, t | 0x08); };
        void _0xcbdf() /* SET 3,A    */ { A |= 0x08; };
        void _0xcbe0() /* SET 4,B    */ { B |= 0x10; };
        void _0xcbe1() /* SET 4,C    */ { C |= 0x10; };
        void _0xcbe2() /* SET 4,D    */ { D |= 0x10; };
        void _0xcbe3() /* SET 4,E    */ { E |= 0x10; };
        void _0xcbe4() /* SET 4,H    */ { H |= 0x10; };
        void _0xcbe5() /* SET 4,L    */ { L |= 0x10; };
        void _0xcbe6() /* SET 4,(HL) */ { uint8_t t = memoryRead(HL); CONTEND_READ_NO_MREQ(HL, 1); memoryWrite(HL, t | 0x10); };
        void _0xcbe7() /* SET 4,A    */ { A |= 0x10; };
        void _0xcbe8() /* SET 5,B    */ { B |= 0x20; };
        void _0xcbe9() /* SET 5,C    */ { C |= 0x20; };
        void _0xcbea() /* SET 5,D    */ { D |= 0x20; };
        void _0xcbeb() /* SET 5,E    */ { E |= 0x20; };
        void _0xcbec() /* SET 5,H    */ { H |= 0x20; };
        void _0xcbed() /* SET 5,L    */ { L |= 0x20; };
        void _0xcbee() /* SET 5,(HL) */ { uint8_t t = memoryRead(HL); CONTEND_READ_NO_MREQ(HL, 1); memoryWrite(HL, t | 0x20); };
        void _0xcbef() /* SET 5,A    */ { A |= 0x20; };
        void _0xcbf0() /* SET 6,B    */ { B |= 0x40; };
        void _0xcbf1() /* SET 6,C    */ { C |= 0x40; };
        void _0xcbf2() /* SET 6,D    */ { D |= 0x40; };
        void _0xcbf3() /* SET 6,E    */ { E |= 0x40; };
        void _0xcbf4() /* SET 6,H    */ { H |= 0x40; };
        void _0xcbf5() /* SET 6,L    */ { L |= 0x40; };
        void _0xcbf6() /* SET 6,(HL) */ { uint8_t t = memoryRead(HL); CONTEND_READ_NO_MREQ(HL, 1); memoryWrite(HL, t | 0x40); };
        void _0xcbf7() /* SET 6,A    */ { A |= 0x40; };
        void _0xcbf8() /* SET 7,B    */ { B |= 0x80; };
        void _0xcbf9() /* SET 7,C    */ { C |= 0x80; };
        void _0xcbfa() /* SET 7,D    */ { D |= 0x80; };
        void _0xcbfb() /* SET 7,E    */ { E |= 0x80; };
        void _0xcbfc() /* SET 7,H    */ { H |= 0x80; };
        void _0xcbfd() /* SET 7,L    */ { L |= 0x80; };
        void _0xcbfe() /* SET 7,(HL) */ { uint8_t t = memoryRead(HL); CONTEND_READ_NO_MREQ(HL, 1); memoryWrite(HL, t | 0x80); };
        void _0xcbff() /* SET 7,A    */ { A |= 0x80; };

        void _RETRY_() /*               */ { --PC; --R; opCode = opCodeDD; retryOpCode = true; };

        void _0xdd09() /* ADD IX,BC     */ { REP_7(CONTEND_READ_NO_MREQ(IR, 1)); ADDW(IX, BC); };
        void _0xdd19() /* ADD IX,DE     */ { REP_7(CONTEND_READ_NO_MREQ(IR, 1)); ADDW(IX, DE); };
        void _0xdd21() /* LD IX,nnnn    */ { IXL = memoryRead(PC++); IXH = memoryRead(PC++); };
        void _0xdd22() /* LD (nnnn),IX  */ LDW_NNRR(IXL, IXH);
        void _0xdd23() /* INC IX        */ { REP_2(CONTEND_READ_NO_MREQ(IR, 1)); ++IX; };
        void _0xdd24() /* INC IXH       */ { INC(IXH); };
        void _0xdd25() /* DEC IXH       */ { DEC(IXH); };
        void _0xdd26() /* LD IXH,nn     */ { IXH = memoryRead(PC++); };
        void _0xdd29() /* ADD IX,IX     */ { REP_7(CONTEND_READ_NO_MREQ(IR, 1)); ADDW(IX, IX); };
        void _0xdd2a() /* LD IX,(nnnn)  */ LDW_RRNN(IXL, IXH);
        void _0xdd2b() /* DEC IX        */ { REP_2(CONTEND_READ_NO_MREQ(IR, 1)); --IX;  };
        void _0xdd2c() /* INC IXL       */ { INC(IXL); };
        void _0xdd2d() /* DEC IXL       */ { DEC(IXL); };
        void _0xdd2e() /* LD IXL,nn     */ { IXL = memoryRead(PC++); };
        void _0xdd34() /* INC (IX+dd)   */ { uint8_t o = memoryRead(PC), t; REP_5(CONTEND_READ_NO_MREQ(PC, 1)); ++PC; MP = IX + (int8_t) o; t = memoryRead(MP); CONTEND_READ_NO_MREQ(MP, 1); INC(t); memoryWrite(MP, t); };
        void _0xdd35() /* DEC (IX+dd)   */ { uint8_t o = memoryRead(PC), t; REP_5(CONTEND_READ_NO_MREQ(PC, 1)); ++PC; MP = IX + (int8_t) o; t = memoryRead(MP); CONTEND_READ_NO_MREQ(MP, 1); DEC(t); memoryWrite(MP, t); };
        void _0xdd36() /* LD (IX+dd),nn */ { uint8_t o = memoryRead(PC++), t = memoryRead(PC); REP_2(CONTEND_READ_NO_MREQ(PC, 1)); ++PC; MP = IX + (int8_t) o; memoryWrite(MP, t); };
        void _0xdd39() /* ADD IX,SP     */ { REP_7(CONTEND_READ_NO_MREQ(IR, 1)); ADDW(IX, SP); };
        void _0xdd44() /* LD B,IXH      */ { B = IXH; };
        void _0xdd45() /* LD B,IXL      */ { B = IXL; };
        void _0xdd46() /* LD B,(IX+dd)  */ { uint8_t o = memoryRead(PC); REP_5(CONTEND_READ_NO_MREQ(PC, 1)); ++PC; MP = IX + (int8_t) o; B = memoryRead(MP); };
        void _0xdd4c() /* LD C,IXH      */ { C = IXH; };
        void _0xdd4d() /* LD C,IXL      */ { C = IXL; };
        void _0xdd4e() /* LD C,(IX+dd)  */ { uint8_t o = memoryRead(PC); REP_5(CONTEND_READ_NO_MREQ(PC, 1)); ++PC; MP = IX + (int8_t) o; C = memoryRead(MP); };
        void _0xdd54() /* LD D,IXH      */ { D = IXH; };
        void _0xdd55() /* LD D,IXL      */ { D = IXL; };
        void _0xdd56() /* LD D,(IX+dd)  */ { uint8_t o = memoryRead(PC); REP_5(CONTEND_READ_NO_MREQ(PC, 1)); ++PC; MP = IX + (int8_t) o; D = memoryRead(MP); };
        void _0xdd5c() /* LD E,IXH      */ { E = IXH; };
        void _0xdd5d() /* LD E,IXL      */ { E = IXL; };
        void _0xdd5e() /* LD E,(IX+dd)  */ { uint8_t o = memoryRead(PC); REP_5(CONTEND_READ_NO_MREQ(PC, 1)); ++PC; MP = IX + (int8_t) o; E = memoryRead(MP); };
        void _0xdd60() /* LD IXH,B      */ { IXH = B; };
        void _0xdd61() /* LD IXH,C      */ { IXH = C; };
        void _0xdd62() /* LD IXH,D      */ { IXH = D; };
        void _0xdd63() /* LD IXH,E      */ { IXH = E; };
        void _0xdd65() /* LD IXH,IXL    */ { IXH = IXL; };
        void _0xdd66() /* LD H,(IX+dd)  */ { uint8_t o = memoryRead(PC); REP_5(CONTEND_READ_NO_MREQ(PC, 1)); ++PC; MP = IX + (int8_t) o; H = memoryRead(MP); };
        void _0xdd67() /* LD IXH,A      */ { IXH = A; };
        void _0xdd68() /* LD IXL,B      */ { IXL = B; };
        void _0xdd69() /* LD IXL,C      */ { IXL = C; };
        void _0xdd6a() /* LD IXL,D      */ { IXL = D; };
        void _0xdd6b() /* LD IXL,E      */ { IXL = E; };
        void _0xdd6c() /* LD IXL,IXH    */ { IXL = IXH; };
        void _0xdd6e() /* LD L,(IX+dd)  */ { uint8_t o = memoryRead(PC); REP_5(CONTEND_READ_NO_MREQ(PC, 1)); ++PC; MP = IX + (int8_t) o; L = memoryRead(MP); };
        void _0xdd6f() /* LD IXL,A      */ { IXL = A; };
        void _0xdd70() /* LD (IX+dd),B  */ { uint8_t o = memoryRead(PC); REP_5(CONTEND_READ_NO_MREQ(PC, 1)); ++PC; MP = IX + (int8_t) o; memoryWrite(MP, B); };
        void _0xdd71() /* LD (IX+dd),C  */ { uint8_t o = memoryRead(PC); REP_5(CONTEND_READ_NO_MREQ(PC, 1)); ++PC; MP = IX + (int8_t) o; memoryWrite(MP, C); };
        void _0xdd72() /* LD (IX+dd),D  */ { uint8_t o = memoryRead(PC); REP_5(CONTEND_READ_NO_MREQ(PC, 1)); ++PC; MP = IX + (int8_t) o; memoryWrite(MP, D); };
        void _0xdd73() /* LD (IX+dd),E  */ { uint8_t o = memoryRead(PC); REP_5(CONTEND_READ_NO_MREQ(PC, 1)); ++PC; MP = IX + (int8_t) o; memoryWrite(MP, E); };
        void _0xdd74() /* LD (IX+dd),H  */ { uint8_t o = memoryRead(PC); REP_5(CONTEND_READ_NO_MREQ(PC, 1)); ++PC; MP = IX + (int8_t) o; memoryWrite(MP, H); };
        void _0xdd75() /* LD (IX+dd),L  */ { uint8_t o = memoryRead(PC); REP_5(CONTEND_READ_NO_MREQ(PC, 1)); ++PC; MP = IX + (int8_t) o; memoryWrite(MP, L); };
        void _0xdd77() /* LD (IX+dd),A  */ { uint8_t o = memoryRead(PC); REP_5(CONTEND_READ_NO_MREQ(PC, 1)); ++PC; MP = IX + (int8_t) o; memoryWrite(MP, A); };
        void _0xdd7c() /* LD A,IXH      */ { A = IXH; };
        void _0xdd7d() /* LD A,IXL      */ { A = IXL; };
        void _0xdd7e() /* LD A,(IX+dd)  */ { uint8_t o = memoryRead(PC); REP_5(CONTEND_READ_NO_MREQ(PC, 1)); ++PC; MP = IX + (int8_t) o; A = memoryRead(MP); };
        void _0xdd84() /* ADD A,IXH     */ ADD(IXH);
        void _0xdd85() /* ADD A,IXL     */ ADD(IXL);;
        void _0xdd86() /* ADD A,(IX+dd) */ { uint8_t o = memoryRead(PC); REP_5(CONTEND_READ_NO_MREQ(PC, 1)); ++PC; MP = IX + (int8_t) o; uint8_t v = memoryRead(MP); ADD(v); };
        void _0xdd8c() /* ADC A,IXH     */ ADC(IXH);
        void _0xdd8d() /* ADC A,IXL     */ ADC(IXL);
        void _0xdd8e() /* ADC A,(IX+dd) */ { uint8_t o = memoryRead(PC); REP_5(CONTEND_READ_NO_MREQ(PC, 1)); ++PC; MP = IX + (int8_t) o; uint8_t v = memoryRead(MP); ADC(v); };
        void _0xdd94() /* SUB A,IXH     */ SUB(IXH);
        void _0xdd95() /* SUB A,IXL     */ SUB(IXL);
        void _0xdd96() /* SUB A,(IX+dd) */ { uint8_t o = memoryRead(PC); REP_5(CONTEND_READ_NO_MREQ(PC, 1)); ++PC; MP = IX + (int8_t) o; uint8_t v = memoryRead(MP); SUB(v); };
        void _0xdd9c() /* SBC A,IXH     */ SBC(IXH);
        void _0xdd9d() /* SBC A,IXL     */ SBC(IXL);
        void _0xdd9e() /* SBC A,(IX+dd) */ { uint8_t o = memoryRead(PC); REP_5(CONTEND_READ_NO_MREQ(PC, 1)); ++PC; MP = IX + (int8_t) o; uint8_t v = memoryRead(MP); SBC(v); };
        void _0xdda4() /* AND A,IXH     */ { AND(IXH); };
        void _0xdda5() /* AND A,IXL     */ { AND(IXL); };
        void _0xdda6() /* AND A,(IX+dd) */ { uint8_t o = memoryRead(PC); REP_5(CONTEND_READ_NO_MREQ(PC, 1)); ++PC; MP = IX + (int8_t) o; uint8_t v = memoryRead(MP); AND(v); };
        void _0xddac() /* XOR A,IXH     */ { XOR(IXH); };
        void _0xddad() /* XOR A,IXL     */ { XOR(IXL); };
        void _0xddae() /* XOR A,(IX+dd) */ { uint8_t o = memoryRead(PC); REP_5(CONTEND_READ_NO_MREQ(PC, 1)); ++PC; MP = IX + (int8_t) o; uint8_t v = memoryRead(MP); XOR(v); };
        void _0xddb4() /* OR A,IXH      */ { OR(IXH); };
        void _0xddb5() /* OR A,IXL      */ { OR(IXL); };
        void _0xddb6() /* OR A,(IX+dd)  */ { uint8_t o = memoryRead(PC); REP_5(CONTEND_READ_NO_MREQ(PC, 1)); ++PC; MP = IX + (int8_t) o; uint8_t v = memoryRead(MP); OR(v); };
        void _0xddbc() /* CP A,IXH      */ CP(IXH);
        void _0xddbd() /* CP A,IXL      */ CP(IXL);
        void _0xddbe() /* CP A,(IX+dd)  */ { uint8_t o = memoryRead(PC); REP_5(CONTEND_READ_NO_MREQ(PC, 1)); ++PC; MP = IX + (int8_t) o; uint8_t v = memoryRead(MP); CP(v); };
        void _0xddcb()                     { CONTEND(PC, 3); MP = IX + (int8_t) memory::read(PC); ++PC; CONTEND(PC, 3); opCodeXXCB = memory::read(PC); REP_2(CONTEND_READ_NO_MREQ(PC, 1)); ++PC; opCodesXXCB[opCodeXXCB](); };
        void _0xdde1() /* POP IX        */ { POPW(IXL, IXH); };
        void _0xdde3() /* EX (SP),IX    */ { uint8_t tl, th; tl = memoryRead(SP); th = memoryRead(SP + 1); CONTEND_READ_NO_MREQ(SP + 1, 1); memoryWrite(SP + 1, IXH); memoryWrite(SP, IXL); REP_2(CONTEND_WRITE_NO_MREQ(SP, 1)); IXL = MPL = tl; IXH = MPH = th; };
        void _0xdde5() /* PUSH IX       */ { CONTEND_READ_NO_MREQ(IR, 1); PUSHW(IXL, IXH); };
        void _0xdde9() /* JP IX         */ { PC = IX; };
        void _0xddf9() /* LD SP,IX      */ { REP_2(CONTEND_READ_NO_MREQ(IR, 1)); SP = IX; };

        void _0xfd09() /* ADD IY,BC     */ { REP_7(CONTEND_READ_NO_MREQ(IR, 1)); ADDW(IY, BC); };
        void _0xfd19() /* ADD IY,DE     */ { REP_7(CONTEND_READ_NO_MREQ(IR, 1)); ADDW(IY, DE); };
        void _0xfd21() /* LD IY,nnnn    */ { IYL = memoryRead(PC++); IYH = memoryRead(PC++); };
        void _0xfd22() /* LD (nnnn),IY  */ LDW_NNRR(IYL, IYH);
        void _0xfd23() /* INC IY        */ { REP_2(CONTEND_READ_NO_MREQ(IR, 1)); ++IY; };
        void _0xfd24() /* INC IYH       */ { INC(IYH); };
        void _0xfd25() /* DEC IYH       */ { DEC(IYH); };
        void _0xfd26() /* LD IYH,nn     */ { IYH = memoryRead(PC++); };
        void _0xfd29() /* ADD IY,IY     */ { REP_7(CONTEND_READ_NO_MREQ(IR, 1)); ADDW(IY, IY); };
        void _0xfd2a() /* LD IY,(nnnn)  */ LDW_RRNN(IYL, IYH);
        void _0xfd2b() /* DEC IY        */ { REP_2(CONTEND_READ_NO_MREQ(IR, 1)); --IY;  };
        void _0xfd2c() /* INC IYL       */ { INC(IYL); };
        void _0xfd2d() /* DEC IYL       */ { DEC(IYL); };
        void _0xfd2e() /* LD IYL,nn     */ { IYL = memoryRead(PC++); };
        void _0xfd34() /* INC (IY+dd)   */ { uint8_t o = memoryRead(PC), t; REP_5(CONTEND_READ_NO_MREQ(PC, 1)); ++PC; MP = IY + (int8_t) o; t = memoryRead(MP); CONTEND_READ_NO_MREQ(MP, 1); INC(t); memoryWrite(MP, t); };
        void _0xfd35() /* DEC (IY+dd)   */ { uint8_t o = memoryRead(PC), t; REP_5(CONTEND_READ_NO_MREQ(PC, 1)); ++PC; MP = IY + (int8_t) o; t = memoryRead(MP); CONTEND_READ_NO_MREQ(MP, 1); DEC(t); memoryWrite(MP, t); };
        void _0xfd36() /* LD (IY+dd),nn */ { uint8_t o = memoryRead(PC++), t = memoryRead(PC); REP_2(CONTEND_READ_NO_MREQ(PC, 1)); ++PC; MP = IY + (int8_t) o; memoryWrite(MP, t); };
        void _0xfd39() /* ADD IY,SP     */ { REP_7(CONTEND_READ_NO_MREQ(IR, 1)); ADDW(IY, SP); };
        void _0xfd44() /* LD B,IYH      */ { B = IYH; };
        void _0xfd45() /* LD B,IYL      */ { B = IYL; };
        void _0xfd46() /* LD B,(IY+dd)  */ { uint8_t o = memoryRead(PC); REP_5(CONTEND_READ_NO_MREQ(PC, 1)); ++PC; MP = IY + (int8_t) o; B = memoryRead(MP); };
        void _0xfd4c() /* LD C,IYH      */ { C = IYH; };
        void _0xfd4d() /* LD C,IYL      */ { C = IYL; };
        void _0xfd4e() /* LD C,(IY+dd)  */ { uint8_t o = memoryRead(PC); REP_5(CONTEND_READ_NO_MREQ(PC, 1)); ++PC; MP = IY + (int8_t) o; C = memoryRead(MP); };
        void _0xfd54() /* LD D,IYH      */ { D = IYH; };
        void _0xfd55() /* LD D,IYL      */ { D = IYL; };
        void _0xfd56() /* LD D,(IY+dd)  */ { uint8_t o = memoryRead(PC); REP_5(CONTEND_READ_NO_MREQ(PC, 1)); ++PC; MP = IY + (int8_t) o; D = memoryRead(MP); };
        void _0xfd5c() /* LD E,IYH      */ { E = IYH; };
        void _0xfd5d() /* LD E,IYL      */ { E = IYL; };
        void _0xfd5e() /* LD E,(IY+dd)  */ { uint8_t o = memoryRead(PC); REP_5(CONTEND_READ_NO_MREQ(PC, 1)); ++PC; MP = IY + (int8_t) o; E = memoryRead(MP); };
        void _0xfd60() /* LD IYH,B      */ { IYH = B; };
        void _0xfd61() /* LD IYH,C      */ { IYH = C; };
        void _0xfd62() /* LD IYH,D      */ { IYH = D; };
        void _0xfd63() /* LD IYH,E      */ { IYH = E; };
        void _0xfd65() /* LD IYH,IYL    */ { IYH = IYL; };
        void _0xfd66() /* LD H,(IY+dd)  */ { uint8_t o = memoryRead(PC); REP_5(CONTEND_READ_NO_MREQ(PC, 1)); ++PC; MP = IY + (int8_t) o; H = memoryRead(MP); };
        void _0xfd67() /* LD IYH,A      */ { IYH = A; };
        void _0xfd68() /* LD IYL,B      */ { IYL = B; };
        void _0xfd69() /* LD IYL,C      */ { IYL = C; };
        void _0xfd6a() /* LD IYL,D      */ { IYL = D; };
        void _0xfd6b() /* LD IYL,E      */ { IYL = E; };
        void _0xfd6c() /* LD IYL,IYH    */ { IYL = IYH; };
        void _0xfd6e() /* LD L,(IY+dd)  */ { uint8_t o = memoryRead(PC); REP_5(CONTEND_READ_NO_MREQ(PC, 1)); ++PC; MP = IY + (int8_t) o; L = memoryRead(MP); };
        void _0xfd6f() /* LD IYL,A      */ { IYL = A; };
        void _0xfd70() /* LD (IY+dd),B  */ { uint8_t o = memoryRead(PC); REP_5(CONTEND_READ_NO_MREQ(PC, 1)); ++PC; MP = IY + (int8_t) o; memoryWrite(MP, B); };
        void _0xfd71() /* LD (IY+dd),C  */ { uint8_t o = memoryRead(PC); REP_5(CONTEND_READ_NO_MREQ(PC, 1)); ++PC; MP = IY + (int8_t) o; memoryWrite(MP, C); };
        void _0xfd72() /* LD (IY+dd),D  */ { uint8_t o = memoryRead(PC); REP_5(CONTEND_READ_NO_MREQ(PC, 1)); ++PC; MP = IY + (int8_t) o; memoryWrite(MP, D); };
        void _0xfd73() /* LD (IY+dd),E  */ { uint8_t o = memoryRead(PC); REP_5(CONTEND_READ_NO_MREQ(PC, 1)); ++PC; MP = IY + (int8_t) o; memoryWrite(MP, E); };
        void _0xfd74() /* LD (IY+dd),H  */ { uint8_t o = memoryRead(PC); REP_5(CONTEND_READ_NO_MREQ(PC, 1)); ++PC; MP = IY + (int8_t) o; memoryWrite(MP, H); };
        void _0xfd75() /* LD (IY+dd),L  */ { uint8_t o = memoryRead(PC); REP_5(CONTEND_READ_NO_MREQ(PC, 1)); ++PC; MP = IY + (int8_t) o; memoryWrite(MP, L); };
        void _0xfd77() /* LD (IY+dd),A  */ { uint8_t o = memoryRead(PC); REP_5(CONTEND_READ_NO_MREQ(PC, 1)); ++PC; MP = IY + (int8_t) o; memoryWrite(MP, A); };
        void _0xfd7c() /* LD A,IYH      */ { A = IYH; };
        void _0xfd7d() /* LD A,IYL      */ { A = IYL; };
        void _0xfd7e() /* LD A,(IY+dd)  */ { uint8_t o = memoryRead(PC); REP_5(CONTEND_READ_NO_MREQ(PC, 1)); ++PC; MP = IY + (int8_t) o; A = memoryRead(MP); };
        void _0xfd84() /* ADD A,IYH     */ ADD(IYH);
        void _0xfd85() /* ADD A,IYL     */ ADD(IYL);
        void _0xfd86() /* ADD A,(IY+dd) */ { uint8_t o = memoryRead(PC); REP_5(CONTEND_READ_NO_MREQ(PC, 1)); ++PC; MP = IY + (int8_t) o; uint8_t v = memoryRead(MP); ADD(v); };
        void _0xfd8c() /* ADC A,IYH     */ ADC(IYH);
        void _0xfd8d() /* ADC A,IYL     */ ADC(IYL);
        void _0xfd8e() /* ADC A,(IY+dd) */ { uint8_t o = memoryRead(PC); REP_5(CONTEND_READ_NO_MREQ(PC, 1)); ++PC; MP = IY + (int8_t) o; uint8_t v = memoryRead(MP); ADC(v); };
        void _0xfd94() /* SUB A,IYH     */ SUB(IYH);
        void _0xfd95() /* SUB A,IYL     */ SUB(IYL);
        void _0xfd96() /* SUB A,(IY+dd) */ { uint8_t o = memoryRead(PC); REP_5(CONTEND_READ_NO_MREQ(PC, 1)); ++PC; MP = IY + (int8_t) o; uint8_t v = memoryRead(MP); SUB(v); };
        void _0xfd9c() /* SBC A,IYH     */ SBC(IYH);
        void _0xfd9d() /* SBC A,IYL     */ SBC(IYL);
        void _0xfd9e() /* SBC A,(IY+dd) */ { uint8_t o = memoryRead(PC); REP_5(CONTEND_READ_NO_MREQ(PC, 1)); ++PC; MP = IY + (int8_t) o; uint8_t v = memoryRead(MP); SBC(v); };
        void _0xfda4() /* AND A,IYH     */ { AND(IYH); };
        void _0xfda5() /* AND A,IYL     */ { AND(IYL); };
        void _0xfda6() /* AND A,(IY+dd) */ { uint8_t o = memoryRead(PC); REP_5(CONTEND_READ_NO_MREQ(PC, 1)); ++PC; MP = IY + (int8_t) o; uint8_t v = memoryRead(MP); AND(v); };
        void _0xfdac() /* XOR A,IYH     */ { XOR(IYH); };
        void _0xfdad() /* XOR A,IYL     */ { XOR(IYL); };
        void _0xfdae() /* XOR A,(IY+dd) */ { uint8_t o = memoryRead(PC); REP_5(CONTEND_READ_NO_MREQ(PC, 1)); ++PC; MP = IY + (int8_t) o; uint8_t v = memoryRead(MP); XOR(v); };
        void _0xfdb4() /* OR A,IYH      */ { OR(IYH); };
        void _0xfdb5() /* OR A,IYL      */ { OR(IYL); };
        void _0xfdb6() /* OR A,(IY+dd)  */ { uint8_t o = memoryRead(PC); REP_5(CONTEND_READ_NO_MREQ(PC, 1)); ++PC; MP = IY + (int8_t) o; uint8_t v = memoryRead(MP); OR(v); };
        void _0xfdbc() /* CP A,IYH      */ CP(IYH);
        void _0xfdbd() /* CP A,IYL      */ CP(IYL);
        void _0xfdbe() /* CP A,(IY+dd)  */ { uint8_t o = memoryRead(PC); REP_5(CONTEND_READ_NO_MREQ(PC, 1)); ++PC; MP = IY + (int8_t) o; uint8_t v = memoryRead(MP); CP(v); };
        void _0xfdcb()                     { CONTEND(PC, 3); MP = IY + (int8_t) memory::read(PC); ++PC; CONTEND(PC, 3); opCodeXXCB = memory::read(PC); REP_2(CONTEND_READ_NO_MREQ(PC, 1)); ++PC; opCodesXXCB[opCodeXXCB](); };
        void _0xfde1() /* POP IY        */ { POPW(IYL, IYH); };
        void _0xfde3() /* EX (SP),IY    */ { uint8_t tl, th; tl = memoryRead(SP); th = memoryRead(SP + 1); CONTEND_READ_NO_MREQ(SP + 1, 1); memoryWrite(SP + 1, IYH); memoryWrite(SP, IYL); REP_2(CONTEND_WRITE_NO_MREQ(SP, 1)); IYL = MPL = tl; IYH = MPH = th; };
        void _0xfde5() /* PUSH IY       */ { CONTEND_READ_NO_MREQ(IR, 1); PUSHW(IYL, IYH); };
        void _0xfde9() /* JP IY         */ { PC = IY; };
        void _0xfdf9() /* LD SP,IY      */ { REP_2(CONTEND_READ_NO_MREQ(IR, 1)); SP = IY; };        

        void _0xxxcb00() /* LD B,RLC (REG+dd)   */ { B = memoryRead(MP); CONTEND_READ_NO_MREQ(MP, 1); RLC(B); memoryWrite(MP, B); };
        void _0xxxcb01() /* LD C,RLC (REG+dd)   */ { C = memoryRead(MP); CONTEND_READ_NO_MREQ(MP, 1); RLC(C); memoryWrite(MP, C); };
        void _0xxxcb02() /* LD D,RLC (REG+dd)   */ { D = memoryRead(MP); CONTEND_READ_NO_MREQ(MP, 1); RLC(D); memoryWrite(MP, D); };
        void _0xxxcb03() /* LD E,RLC (REG+dd)   */ { E = memoryRead(MP); CONTEND_READ_NO_MREQ(MP, 1); RLC(E); memoryWrite(MP, E); };
        void _0xxxcb04() /* LD H,RLC (REG+dd)   */ { H = memoryRead(MP); CONTEND_READ_NO_MREQ(MP, 1); RLC(H); memoryWrite(MP, H); };
        void _0xxxcb05() /* LD L,RLC (REG+dd)   */ { L = memoryRead(MP); CONTEND_READ_NO_MREQ(MP, 1); RLC(L); memoryWrite(MP, L); };
        void _0xxxcb06() /* RLC (REG+dd)        */ { uint8_t t = memoryRead(MP); CONTEND_READ_NO_MREQ(MP, 1); RLC(t); memoryWrite(MP, t); };
        void _0xxxcb07() /* LD A,RLC (REG+dd)   */ { A = memoryRead(MP); CONTEND_READ_NO_MREQ(MP, 1); RLC(A); memoryWrite(MP, A); };
        void _0xxxcb08() /* LD B,RRC (REG+dd)   */ { B = memoryRead(MP); CONTEND_READ_NO_MREQ(MP, 1); RRC(B); memoryWrite(MP, B); };
        void _0xxxcb09() /* LD C,RRC (REG+dd)   */ { C = memoryRead(MP); CONTEND_READ_NO_MREQ(MP, 1); RRC(C); memoryWrite(MP, C); };
        void _0xxxcb0a() /* LD D,RRC (REG+dd)   */ { D = memoryRead(MP); CONTEND_READ_NO_MREQ(MP, 1); RRC(D); memoryWrite(MP, D); };
        void _0xxxcb0b() /* LD E,RRC (REG+dd)   */ { E = memoryRead(MP); CONTEND_READ_NO_MREQ(MP, 1); RRC(E); memoryWrite(MP, E); };
        void _0xxxcb0c() /* LD H,RRC (REG+dd)   */ { H = memoryRead(MP); CONTEND_READ_NO_MREQ(MP, 1); RRC(H); memoryWrite(MP, H); };
        void _0xxxcb0d() /* LD L,RRC (REG+dd)   */ { L = memoryRead(MP); CONTEND_READ_NO_MREQ(MP, 1); RRC(L); memoryWrite(MP, L); };
        void _0xxxcb0e() /* RRC (REG+dd)        */ { uint8_t t = memoryRead(MP); CONTEND_READ_NO_MREQ(MP, 1); RRC(t); memoryWrite(MP, t); };
        void _0xxxcb0f() /* LD A,RRC (REG+dd)   */ { A = memoryRead(MP); CONTEND_READ_NO_MREQ(MP, 1); RRC(A); memoryWrite(MP, A); };
        void _0xxxcb10() /* LD B,RL (REG+dd)    */ { B = memoryRead(MP); CONTEND_READ_NO_MREQ(MP, 1); RL(B); memoryWrite(MP, B); };
        void _0xxxcb11() /* LD C,RL (REG+dd)    */ { C = memoryRead(MP); CONTEND_READ_NO_MREQ(MP, 1); RL(C); memoryWrite(MP, C); };
        void _0xxxcb12() /* LD D,RL (REG+dd)    */ { D = memoryRead(MP); CONTEND_READ_NO_MREQ(MP, 1); RL(D); memoryWrite(MP, D); };
        void _0xxxcb13() /* LD E,RL (REG+dd)    */ { E = memoryRead(MP); CONTEND_READ_NO_MREQ(MP, 1); RL(E); memoryWrite(MP, E); };
        void _0xxxcb14() /* LD H,RL (REG+dd)    */ { H = memoryRead(MP); CONTEND_READ_NO_MREQ(MP, 1); RL(H); memoryWrite(MP, H); };
        void _0xxxcb15() /* LD L,RL (REG+dd)    */ { L = memoryRead(MP); CONTEND_READ_NO_MREQ(MP, 1); RL(L); memoryWrite(MP, L); };
        void _0xxxcb16() /* RL (REG+dd)         */ { uint8_t t = memoryRead(MP); CONTEND_READ_NO_MREQ(MP, 1); RL(t); memoryWrite(MP, t); };
        void _0xxxcb17() /* LD A,RL (REG+dd)    */ { A = memoryRead(MP); CONTEND_READ_NO_MREQ(MP, 1); RL(A); memoryWrite(MP, A); };
        void _0xxxcb18() /* LD B,RR (REG+dd)    */ { B = memoryRead(MP); CONTEND_READ_NO_MREQ(MP, 1); RR(B); memoryWrite(MP, B); };
        void _0xxxcb19() /* LD C,RR (REG+dd)    */ { C = memoryRead(MP); CONTEND_READ_NO_MREQ(MP, 1); RR(C); memoryWrite(MP, C); };
        void _0xxxcb1a() /* LD D,RR (REG+dd)    */ { D = memoryRead(MP); CONTEND_READ_NO_MREQ(MP, 1); RR(D); memoryWrite(MP, D); };
        void _0xxxcb1b() /* LD E,RR (REG+dd)    */ { E = memoryRead(MP); CONTEND_READ_NO_MREQ(MP, 1); RR(E); memoryWrite(MP, E); };
        void _0xxxcb1c() /* LD H,RR (REG+dd)    */ { H = memoryRead(MP); CONTEND_READ_NO_MREQ(MP, 1); RR(H); memoryWrite(MP, H); };
        void _0xxxcb1d() /* LD L,RR (REG+dd)    */ { L = memoryRead(MP); CONTEND_READ_NO_MREQ(MP, 1); RR(L); memoryWrite(MP, L); };
        void _0xxxcb1e() /* RR (REG+dd)         */ { uint8_t t = memoryRead(MP); CONTEND_READ_NO_MREQ(MP, 1); RR(t); memoryWrite(MP, t); };
        void _0xxxcb1f() /* LD A,RR (REG+dd)    */ { A = memoryRead(MP); CONTEND_READ_NO_MREQ(MP, 1); RR(A); memoryWrite(MP, A); };
        void _0xxxcb20() /* LD B,SLA (REG+dd)   */ { B = memoryRead(MP); CONTEND_READ_NO_MREQ(MP, 1); SLA(B); memoryWrite(MP, B); };
        void _0xxxcb21() /* LD C,SLA (REG+dd)   */ { C = memoryRead(MP); CONTEND_READ_NO_MREQ(MP, 1); SLA(C); memoryWrite(MP, C); };
        void _0xxxcb22() /* LD D,SLA (REG+dd)   */ { D = memoryRead(MP); CONTEND_READ_NO_MREQ(MP, 1); SLA(D); memoryWrite(MP, D); };
        void _0xxxcb23() /* LD E,SLA (REG+dd)   */ { E = memoryRead(MP); CONTEND_READ_NO_MREQ(MP, 1); SLA(E); memoryWrite(MP, E); };
        void _0xxxcb24() /* LD H,SLA (REG+dd)   */ { H = memoryRead(MP); CONTEND_READ_NO_MREQ(MP, 1); SLA(H); memoryWrite(MP, H); };
        void _0xxxcb25() /* LD L,SLA (REG+dd)   */ { L = memoryRead(MP); CONTEND_READ_NO_MREQ(MP, 1); SLA(L); memoryWrite(MP, L); };
        void _0xxxcb26() /* SLA (REG+dd)        */ { uint8_t t = memoryRead(MP); CONTEND_READ_NO_MREQ(MP, 1); SLA(t); memoryWrite(MP, t); };
        void _0xxxcb27() /* LD A,SLA (REG+dd)   */ { A = memoryRead(MP); CONTEND_READ_NO_MREQ(MP, 1); SLA(A); memoryWrite(MP, A); };
        void _0xxxcb28() /* LD B,SRA (REG+dd)   */ { B = memoryRead(MP); CONTEND_READ_NO_MREQ(MP, 1); SRA(B); memoryWrite(MP, B); };
        void _0xxxcb29() /* LD C,SRA (REG+dd)   */ { C = memoryRead(MP); CONTEND_READ_NO_MREQ(MP, 1); SRA(C); memoryWrite(MP, C); };
        void _0xxxcb2a() /* LD D,SRA (REG+dd)   */ { D = memoryRead(MP); CONTEND_READ_NO_MREQ(MP, 1); SRA(D); memoryWrite(MP, D); };
        void _0xxxcb2b() /* LD E,SRA (REG+dd)   */ { E = memoryRead(MP); CONTEND_READ_NO_MREQ(MP, 1); SRA(E); memoryWrite(MP, E); };
        void _0xxxcb2c() /* LD H,SRA (REG+dd)   */ { H = memoryRead(MP); CONTEND_READ_NO_MREQ(MP, 1); SRA(H); memoryWrite(MP, H); };
        void _0xxxcb2d() /* LD L,SRA (REG+dd)   */ { L = memoryRead(MP); CONTEND_READ_NO_MREQ(MP, 1); SRA(L); memoryWrite(MP, L); };
        void _0xxxcb2e() /* SRA (REG+dd)        */ { uint8_t t = memoryRead(MP); CONTEND_READ_NO_MREQ(MP, 1); SRA(t); memoryWrite(MP, t); };
        void _0xxxcb2f() /* LD A,SRA (REG+dd)   */ { A = memoryRead(MP); CONTEND_READ_NO_MREQ(MP, 1); SRA(A); memoryWrite(MP, A); };
        void _0xxxcb30() /* LD B,SLL (REG+dd)   */ { B = memoryRead(MP); CONTEND_READ_NO_MREQ(MP, 1); SLL(B); memoryWrite(MP, B); };
        void _0xxxcb31() /* LD C,SLL (REG+dd)   */ { C = memoryRead(MP); CONTEND_READ_NO_MREQ(MP, 1); SLL(C); memoryWrite(MP, C); };
        void _0xxxcb32() /* LD D,SLL (REG+dd)   */ { D = memoryRead(MP); CONTEND_READ_NO_MREQ(MP, 1); SLL(D); memoryWrite(MP, D); };
        void _0xxxcb33() /* LD E,SLL (REG+dd)   */ { E = memoryRead(MP); CONTEND_READ_NO_MREQ(MP, 1); SLL(E); memoryWrite(MP, E); };
        void _0xxxcb34() /* LD H,SLL (REG+dd)   */ { H = memoryRead(MP); CONTEND_READ_NO_MREQ(MP, 1); SLL(H); memoryWrite(MP, H); };
        void _0xxxcb35() /* LD L,SLL (REG+dd)   */ { L = memoryRead(MP); CONTEND_READ_NO_MREQ(MP, 1); SLL(L); memoryWrite(MP, L); };
        void _0xxxcb36() /* SLL (REG+dd)        */ { uint8_t t = memoryRead(MP); CONTEND_READ_NO_MREQ(MP, 1); SLL(t); memoryWrite(MP, t); };
        void _0xxxcb37() /* LD A,SLL (REG+dd)   */ { A = memoryRead(MP); CONTEND_READ_NO_MREQ(MP, 1); SLL(A); memoryWrite(MP, A); };
        void _0xxxcb38() /* LD B,SRL (REG+dd)   */ { B = memoryRead(MP); CONTEND_READ_NO_MREQ(MP, 1); SRL(B); memoryWrite(MP, B); };
        void _0xxxcb39() /* LD C,SRL (REG+dd)   */ { C = memoryRead(MP); CONTEND_READ_NO_MREQ(MP, 1); SRL(C); memoryWrite(MP, C); };
        void _0xxxcb3a() /* LD D,SRL (REG+dd)   */ { D = memoryRead(MP); CONTEND_READ_NO_MREQ(MP, 1); SRL(D); memoryWrite(MP, D); };
        void _0xxxcb3b() /* LD E,SRL (REG+dd)   */ { E = memoryRead(MP); CONTEND_READ_NO_MREQ(MP, 1); SRL(E); memoryWrite(MP, E); };
        void _0xxxcb3c() /* LD H,SRL (REG+dd)   */ { H = memoryRead(MP); CONTEND_READ_NO_MREQ(MP, 1); SRL(H); memoryWrite(MP, H); };
        void _0xxxcb3d() /* LD L,SRL (REG+dd)   */ { L = memoryRead(MP); CONTEND_READ_NO_MREQ(MP, 1); SRL(L); memoryWrite(MP, L); };
        void _0xxxcb3e() /* SRL (REG+dd)        */ { uint8_t t = memoryRead(MP); CONTEND_READ_NO_MREQ(MP, 1); SRL(t); memoryWrite(MP, t); };
        void _0xxxcb3f() /* LD A,SRL (REG+dd)   */ { A = memoryRead(MP); CONTEND_READ_NO_MREQ(MP, 1); SRL(A); memoryWrite(MP, A); };
        void _0xxxcb47() /* BIT 0,(REG+dd)      */ { uint8_t t = memoryRead(MP); CONTEND_READ_NO_MREQ(MP, 1); BIT_MP(0, t); };
        void _0xxxcb4f() /* BIT 1,(REG+dd)      */ { uint8_t t = memoryRead(MP); CONTEND_READ_NO_MREQ(MP, 1); BIT_MP(1, t); };
        void _0xxxcb57() /* BIT 2,(REG+dd)      */ { uint8_t t = memoryRead(MP); CONTEND_READ_NO_MREQ(MP, 1); BIT_MP(2, t); };
        void _0xxxcb5f() /* BIT 3,(REG+dd)      */ { uint8_t t = memoryRead(MP); CONTEND_READ_NO_MREQ(MP, 1); BIT_MP(3, t); };
        void _0xxxcb67() /* BIT 4,(REG+dd)      */ { uint8_t t = memoryRead(MP); CONTEND_READ_NO_MREQ(MP, 1); BIT_MP(4, t); };
        void _0xxxcb6f() /* BIT 5,(REG+dd)      */ { uint8_t t = memoryRead(MP); CONTEND_READ_NO_MREQ(MP, 1); BIT_MP(5, t); };
        void _0xxxcb77() /* BIT 6,(REG+dd)      */ { uint8_t t = memoryRead(MP); CONTEND_READ_NO_MREQ(MP, 1); BIT_MP(6, t); };
        void _0xxxcb7f() /* BIT 7,(REG+dd)      */ { uint8_t t = memoryRead(MP); CONTEND_READ_NO_MREQ(MP, 1); BIT_MP(7, t); };
        void _0xxxcb80() /* LD B,RES 0,(REG+dd) */ { B = memoryRead(MP) & 0xfe; CONTEND_READ_NO_MREQ(MP, 1); memoryWrite(MP, B); };
        void _0xxxcb81() /* LD C,RES 0,(REG+dd) */ { C = memoryRead(MP) & 0xfe; CONTEND_READ_NO_MREQ(MP, 1); memoryWrite(MP, C); };
        void _0xxxcb82() /* LD D,RES 0,(REG+dd) */ { D = memoryRead(MP) & 0xfe; CONTEND_READ_NO_MREQ(MP, 1); memoryWrite(MP, D); };
        void _0xxxcb83() /* LD E,RES 0,(REG+dd) */ { E = memoryRead(MP) & 0xfe; CONTEND_READ_NO_MREQ(MP, 1); memoryWrite(MP, E); };
        void _0xxxcb84() /* LD H,RES 0,(REG+dd) */ { H = memoryRead(MP) & 0xfe; CONTEND_READ_NO_MREQ(MP, 1); memoryWrite(MP, H); };
        void _0xxxcb85() /* LD L,RES 0,(REG+dd) */ { L = memoryRead(MP) & 0xfe; CONTEND_READ_NO_MREQ(MP, 1); memoryWrite(MP, L); };
        void _0xxxcb86() /* RES 0,(REG+dd)      */ { uint8_t t = memoryRead(MP); CONTEND_READ_NO_MREQ(MP, 1); memoryWrite(MP, t & 0xfe); };
        void _0xxxcb87() /* LD A,RES 0,(REG+dd) */ { A = memoryRead(MP) & 0xfe; CONTEND_READ_NO_MREQ(MP, 1); memoryWrite(MP, A); };
        void _0xxxcb88() /* LD B,RES 1,(REG+dd) */ { B = memoryRead(MP) & 0xfd; CONTEND_READ_NO_MREQ(MP, 1); memoryWrite(MP, B); };
        void _0xxxcb89() /* LD C,RES 1,(REG+dd) */ { C = memoryRead(MP) & 0xfd; CONTEND_READ_NO_MREQ(MP, 1); memoryWrite(MP, C); };
        void _0xxxcb8a() /* LD D,RES 1,(REG+dd) */ { D = memoryRead(MP) & 0xfd; CONTEND_READ_NO_MREQ(MP, 1); memoryWrite(MP, D); };
        void _0xxxcb8b() /* LD E,RES 1,(REG+dd) */ { E = memoryRead(MP) & 0xfd; CONTEND_READ_NO_MREQ(MP, 1); memoryWrite(MP, E); };
        void _0xxxcb8c() /* LD H,RES 1,(REG+dd) */ { H = memoryRead(MP) & 0xfd; CONTEND_READ_NO_MREQ(MP, 1); memoryWrite(MP, H); };
        void _0xxxcb8d() /* LD L,RES 1,(REG+dd) */ { L = memoryRead(MP) & 0xfd; CONTEND_READ_NO_MREQ(MP, 1); memoryWrite(MP, L); };
        void _0xxxcb8e() /* RES 1,(REG+dd)      */ { uint8_t t = memoryRead(MP); CONTEND_READ_NO_MREQ(MP, 1); memoryWrite(MP, t & 0xfd); };
        void _0xxxcb8f() /* LD A,RES 1,(REG+dd) */ { A = memoryRead(MP) & 0xfd; CONTEND_READ_NO_MREQ(MP, 1); memoryWrite(MP, A); };
        void _0xxxcb90() /* LD B,RES 2,(REG+dd) */ { B = memoryRead(MP) & 0xfb; CONTEND_READ_NO_MREQ(MP, 1); memoryWrite(MP, B); };
        void _0xxxcb91() /* LD C,RES 2,(REG+dd) */ { C = memoryRead(MP) & 0xfb; CONTEND_READ_NO_MREQ(MP, 1); memoryWrite(MP, C); };
        void _0xxxcb92() /* LD D,RES 2,(REG+dd) */ { D = memoryRead(MP) & 0xfb; CONTEND_READ_NO_MREQ(MP, 1); memoryWrite(MP, D); };
        void _0xxxcb93() /* LD E,RES 2,(REG+dd) */ { E = memoryRead(MP) & 0xfb; CONTEND_READ_NO_MREQ(MP, 1); memoryWrite(MP, E); };
        void _0xxxcb94() /* LD H,RES 2,(REG+dd) */ { H = memoryRead(MP) & 0xfb; CONTEND_READ_NO_MREQ(MP, 1); memoryWrite(MP, H); };
        void _0xxxcb95() /* LD L,RES 2,(REG+dd) */ { L = memoryRead(MP) & 0xfb; CONTEND_READ_NO_MREQ(MP, 1); memoryWrite(MP, L); };
        void _0xxxcb96() /* RES 2,(REG+dd)      */ { uint8_t t = memoryRead(MP); CONTEND_READ_NO_MREQ(MP, 1); memoryWrite(MP, t & 0xfb); };
        void _0xxxcb97() /* LD A,RES 2,(REG+dd) */ { A = memoryRead(MP) & 0xfb; CONTEND_READ_NO_MREQ(MP, 1); memoryWrite(MP, A); };
        void _0xxxcb98() /* LD B,RES 3,(REG+dd) */ { B = memoryRead(MP) & 0xf7; CONTEND_READ_NO_MREQ(MP, 1); memoryWrite(MP, B); };
        void _0xxxcb99() /* LD C,RES 3,(REG+dd) */ { C = memoryRead(MP) & 0xf7; CONTEND_READ_NO_MREQ(MP, 1); memoryWrite(MP, C); };
        void _0xxxcb9a() /* LD D,RES 3,(REG+dd) */ { D = memoryRead(MP) & 0xf7; CONTEND_READ_NO_MREQ(MP, 1); memoryWrite(MP, D); };
        void _0xxxcb9b() /* LD E,RES 3,(REG+dd) */ { E = memoryRead(MP) & 0xf7; CONTEND_READ_NO_MREQ(MP, 1); memoryWrite(MP, E); };
        void _0xxxcb9c() /* LD H,RES 3,(REG+dd) */ { H = memoryRead(MP) & 0xf7; CONTEND_READ_NO_MREQ(MP, 1); memoryWrite(MP, H); };
        void _0xxxcb9d() /* LD L,RES 3,(REG+dd) */ { L = memoryRead(MP) & 0xf7; CONTEND_READ_NO_MREQ(MP, 1); memoryWrite(MP, L); };
        void _0xxxcb9e() /* RES 3,(REG+dd)      */ { uint8_t t = memoryRead(MP); CONTEND_READ_NO_MREQ(MP, 1); memoryWrite(MP, t & 0xf7); };
        void _0xxxcb9f() /* LD A,RES 3,(REG+dd) */ { A = memoryRead(MP) & 0xf7; CONTEND_READ_NO_MREQ(MP, 1); memoryWrite(MP, A); };
        void _0xxxcba0() /* LD B,RES 4,(REG+dd) */ { B = memoryRead(MP) & 0xef; CONTEND_READ_NO_MREQ(MP, 1); memoryWrite(MP, B); };
        void _0xxxcba1() /* LD C,RES 4,(REG+dd) */ { C = memoryRead(MP) & 0xef; CONTEND_READ_NO_MREQ(MP, 1); memoryWrite(MP, C); };
        void _0xxxcba2() /* LD D,RES 4,(REG+dd) */ { D = memoryRead(MP) & 0xef; CONTEND_READ_NO_MREQ(MP, 1); memoryWrite(MP, D); };
        void _0xxxcba3() /* LD E,RES 4,(REG+dd) */ { E = memoryRead(MP) & 0xef; CONTEND_READ_NO_MREQ(MP, 1); memoryWrite(MP, E); };
        void _0xxxcba4() /* LD H,RES 4,(REG+dd) */ { H = memoryRead(MP) & 0xef; CONTEND_READ_NO_MREQ(MP, 1); memoryWrite(MP, H); };
        void _0xxxcba5() /* LD L,RES 4,(REG+dd) */ { L = memoryRead(MP) & 0xef; CONTEND_READ_NO_MREQ(MP, 1); memoryWrite(MP, L); };
        void _0xxxcba6() /* RES 4,(REG+dd)      */ { uint8_t t = memoryRead(MP); CONTEND_READ_NO_MREQ(MP, 1); memoryWrite(MP, t & 0xef); };
        void _0xxxcba7() /* LD A,RES 4,(REG+dd) */ { A = memoryRead(MP) & 0xef; CONTEND_READ_NO_MREQ(MP, 1); memoryWrite(MP, A); };
        void _0xxxcba8() /* LD B,RES 5,(REG+dd) */ { B = memoryRead(MP) & 0xdf; CONTEND_READ_NO_MREQ(MP, 1); memoryWrite(MP, B); };
        void _0xxxcba9() /* LD C,RES 5,(REG+dd) */ { C = memoryRead(MP) & 0xdf; CONTEND_READ_NO_MREQ(MP, 1); memoryWrite(MP, C); };
        void _0xxxcbaa() /* LD D,RES 5,(REG+dd) */ { D = memoryRead(MP) & 0xdf; CONTEND_READ_NO_MREQ(MP, 1); memoryWrite(MP, D); };
        void _0xxxcbab() /* LD E,RES 5,(REG+dd) */ { E = memoryRead(MP) & 0xdf; CONTEND_READ_NO_MREQ(MP, 1); memoryWrite(MP, E); };
        void _0xxxcbac() /* LD H,RES 5,(REG+dd) */ { H = memoryRead(MP) & 0xdf; CONTEND_READ_NO_MREQ(MP, 1); memoryWrite(MP, H); };
        void _0xxxcbad() /* LD L,RES 5,(REG+dd) */ { L = memoryRead(MP) & 0xdf; CONTEND_READ_NO_MREQ(MP, 1); memoryWrite(MP, L); };
        void _0xxxcbae() /* RES 5,(REG+dd)      */ { uint8_t t = memoryRead(MP); CONTEND_READ_NO_MREQ(MP, 1); memoryWrite(MP, t & 0xdf); };
        void _0xxxcbaf() /* LD A,RES 5,(REG+dd) */ { A = memoryRead(MP) & 0xdf; CONTEND_READ_NO_MREQ(MP, 1); memoryWrite(MP, A); };
        void _0xxxcbb0() /* LD B,RES 6,(REG+dd) */ { B = memoryRead(MP) & 0xbf; CONTEND_READ_NO_MREQ(MP, 1); memoryWrite(MP, B); };
        void _0xxxcbb1() /* LD C,RES 6,(REG+dd) */ { C = memoryRead(MP) & 0xbf; CONTEND_READ_NO_MREQ(MP, 1); memoryWrite(MP, C); };
        void _0xxxcbb2() /* LD D,RES 6,(REG+dd) */ { D = memoryRead(MP) & 0xbf; CONTEND_READ_NO_MREQ(MP, 1); memoryWrite(MP, D); };
        void _0xxxcbb3() /* LD E,RES 6,(REG+dd) */ { E = memoryRead(MP) & 0xbf; CONTEND_READ_NO_MREQ(MP, 1); memoryWrite(MP, E); };
        void _0xxxcbb4() /* LD H,RES 6,(REG+dd) */ { H = memoryRead(MP) & 0xbf; CONTEND_READ_NO_MREQ(MP, 1); memoryWrite(MP, H); };
        void _0xxxcbb5() /* LD L,RES 6,(REG+dd) */ { L = memoryRead(MP) & 0xbf; CONTEND_READ_NO_MREQ(MP, 1); memoryWrite(MP, L); };
        void _0xxxcbb6() /* RES 6,(REG+dd)      */ { uint8_t t = memoryRead(MP); CONTEND_READ_NO_MREQ(MP, 1); memoryWrite(MP, t & 0xbf); };
        void _0xxxcbb7() /* LD A,RES 6,(REG+dd) */ { A = memoryRead(MP) & 0xbf; CONTEND_READ_NO_MREQ(MP, 1); memoryWrite(MP, A); };
        void _0xxxcbb8() /* LD B,RES 7,(REG+dd) */ { B = memoryRead(MP) & 0x7f; CONTEND_READ_NO_MREQ(MP, 1); memoryWrite(MP, B); };
        void _0xxxcbb9() /* LD C,RES 7,(REG+dd) */ { C = memoryRead(MP) & 0x7f; CONTEND_READ_NO_MREQ(MP, 1); memoryWrite(MP, C); };
        void _0xxxcbba() /* LD D,RES 7,(REG+dd) */ { D = memoryRead(MP) & 0x7f; CONTEND_READ_NO_MREQ(MP, 1); memoryWrite(MP, D); };
        void _0xxxcbbb() /* LD E,RES 7,(REG+dd) */ { E = memoryRead(MP) & 0x7f; CONTEND_READ_NO_MREQ(MP, 1); memoryWrite(MP, E); };
        void _0xxxcbbc() /* LD H,RES 7,(REG+dd) */ { H = memoryRead(MP) & 0x7f; CONTEND_READ_NO_MREQ(MP, 1); memoryWrite(MP, H); };
        void _0xxxcbbd() /* LD L,RES 7,(REG+dd) */ { L = memoryRead(MP) & 0x7f; CONTEND_READ_NO_MREQ(MP, 1); memoryWrite(MP, L); };
        void _0xxxcbbe() /* RES 7,(REG+dd)      */ { uint8_t t = memoryRead(MP); CONTEND_READ_NO_MREQ(MP, 1); memoryWrite(MP, t & 0x7f); };
        void _0xxxcbbf() /* LD A,RES 7,(REG+dd) */ { A = memoryRead(MP) & 0x7f; CONTEND_READ_NO_MREQ(MP, 1); memoryWrite(MP, A); };
        void _0xxxcbc0() /* LD B,SET 0,(REG+dd) */ { B = memoryRead(MP) | 0x01; CONTEND_READ_NO_MREQ(MP, 1); memoryWrite(MP, B); };
        void _0xxxcbc1() /* LD C,SET 0,(REG+dd) */ { C = memoryRead(MP) | 0x01; CONTEND_READ_NO_MREQ(MP, 1); memoryWrite(MP, C); };
        void _0xxxcbc2() /* LD D,SET 0,(REG+dd) */ { D = memoryRead(MP) | 0x01; CONTEND_READ_NO_MREQ(MP, 1); memoryWrite(MP, D); };
        void _0xxxcbc3() /* LD E,SET 0,(REG+dd) */ { E = memoryRead(MP) | 0x01; CONTEND_READ_NO_MREQ(MP, 1); memoryWrite(MP, E); };
        void _0xxxcbc4() /* LD H,SET 0,(REG+dd) */ { H = memoryRead(MP) | 0x01; CONTEND_READ_NO_MREQ(MP, 1); memoryWrite(MP, H); };
        void _0xxxcbc5() /* LD L,SET 0,(REG+dd) */ { L = memoryRead(MP) | 0x01; CONTEND_READ_NO_MREQ(MP, 1); memoryWrite(MP, L); };
        void _0xxxcbc6() /* SET 0,(REG+dd)      */ { uint8_t t = memoryRead(MP); CONTEND_READ_NO_MREQ(MP, 1); memoryWrite(MP, t | 0x01); };
        void _0xxxcbc7() /* LD A,SET 0,(REG+dd) */ { A = memoryRead(MP) | 0x01; CONTEND_READ_NO_MREQ(MP, 1); memoryWrite(MP, A); };
        void _0xxxcbc8() /* LD B,SET 1,(REG+dd) */ { B = memoryRead(MP) | 0x02; CONTEND_READ_NO_MREQ(MP, 1); memoryWrite(MP, B); };
        void _0xxxcbc9() /* LD C,SET 1,(REG+dd) */ { C = memoryRead(MP) | 0x02; CONTEND_READ_NO_MREQ(MP, 1); memoryWrite(MP, C); };
        void _0xxxcbca() /* LD D,SET 1,(REG+dd) */ { D = memoryRead(MP) | 0x02; CONTEND_READ_NO_MREQ(MP, 1); memoryWrite(MP, D); };
        void _0xxxcbcb() /* LD E,SET 1,(REG+dd) */ { E = memoryRead(MP) | 0x02; CONTEND_READ_NO_MREQ(MP, 1); memoryWrite(MP, E); };
        void _0xxxcbcc() /* LD H,SET 1,(REG+dd) */ { H = memoryRead(MP) | 0x02; CONTEND_READ_NO_MREQ(MP, 1); memoryWrite(MP, H); };
        void _0xxxcbcd() /* LD L,SET 1,(REG+dd) */ { L = memoryRead(MP) | 0x02; CONTEND_READ_NO_MREQ(MP, 1); memoryWrite(MP, L); };
        void _0xxxcbce() /* SET 1,(REG+dd)      */ { uint8_t t = memoryRead(MP); CONTEND_READ_NO_MREQ(MP, 1); memoryWrite(MP, t | 0x02); };
        void _0xxxcbcf() /* LD A,SET 1,(REG+dd) */ { A = memoryRead(MP) | 0x02; CONTEND_READ_NO_MREQ(MP, 1); memoryWrite(MP, A); };
        void _0xxxcbd0() /* LD B,SET 2,(REG+dd) */ { B = memoryRead(MP) | 0x04; CONTEND_READ_NO_MREQ(MP, 1); memoryWrite(MP, B); };
        void _0xxxcbd1() /* LD C,SET 2,(REG+dd) */ { C = memoryRead(MP) | 0x04; CONTEND_READ_NO_MREQ(MP, 1); memoryWrite(MP, C); };
        void _0xxxcbd2() /* LD D,SET 2,(REG+dd) */ { D = memoryRead(MP) | 0x04; CONTEND_READ_NO_MREQ(MP, 1); memoryWrite(MP, D); };
        void _0xxxcbd3() /* LD E,SET 2,(REG+dd) */ { E = memoryRead(MP) | 0x04; CONTEND_READ_NO_MREQ(MP, 1); memoryWrite(MP, E); };
        void _0xxxcbd4() /* LD H,SET 2,(REG+dd) */ { H = memoryRead(MP) | 0x04; CONTEND_READ_NO_MREQ(MP, 1); memoryWrite(MP, H); };
        void _0xxxcbd5() /* LD L,SET 2,(REG+dd) */ { L = memoryRead(MP) | 0x04; CONTEND_READ_NO_MREQ(MP, 1); memoryWrite(MP, L); };
        void _0xxxcbd6() /* SET 2,(REG+dd)      */ { uint8_t t = memoryRead(MP); CONTEND_READ_NO_MREQ(MP, 1); memoryWrite(MP, t | 0x04); };
        void _0xxxcbd7() /* LD A,SET 2,(REG+dd) */ { A = memoryRead(MP) | 0x04; CONTEND_READ_NO_MREQ(MP, 1); memoryWrite(MP, A); };
        void _0xxxcbd8() /* LD B,SET 3,(REG+dd) */ { B = memoryRead(MP) | 0x08; CONTEND_READ_NO_MREQ(MP, 1); memoryWrite(MP, B); };
        void _0xxxcbd9() /* LD C,SET 3,(REG+dd) */ { C = memoryRead(MP) | 0x08; CONTEND_READ_NO_MREQ(MP, 1); memoryWrite(MP, C); };
        void _0xxxcbda() /* LD D,SET 3,(REG+dd) */ { D = memoryRead(MP) | 0x08; CONTEND_READ_NO_MREQ(MP, 1); memoryWrite(MP, D); };
        void _0xxxcbdb() /* LD E,SET 3,(REG+dd) */ { E = memoryRead(MP) | 0x08; CONTEND_READ_NO_MREQ(MP, 1); memoryWrite(MP, E); };
        void _0xxxcbdc() /* LD H,SET 3,(REG+dd) */ { H = memoryRead(MP) | 0x08; CONTEND_READ_NO_MREQ(MP, 1); memoryWrite(MP, H); };
        void _0xxxcbdd() /* LD L,SET 3,(REG+dd) */ { L = memoryRead(MP) | 0x08; CONTEND_READ_NO_MREQ(MP, 1); memoryWrite(MP, L); };
        void _0xxxcbde() /* SET 3,(REG+dd)      */ { uint8_t t = memoryRead(MP); CONTEND_READ_NO_MREQ(MP, 1); memoryWrite(MP, t | 0x08); };
        void _0xxxcbdf() /* LD A,SET 3,(REG+dd) */ { A = memoryRead(MP) | 0x08; CONTEND_READ_NO_MREQ(MP, 1); memoryWrite(MP, A); };
        void _0xxxcbe0() /* LD B,SET 4,(REG+dd) */ { B = memoryRead(MP) | 0x10; CONTEND_READ_NO_MREQ(MP, 1); memoryWrite(MP, B); };
        void _0xxxcbe1() /* LD C,SET 4,(REG+dd) */ { C = memoryRead(MP) | 0x10; CONTEND_READ_NO_MREQ(MP, 1); memoryWrite(MP, C); };
        void _0xxxcbe2() /* LD D,SET 4,(REG+dd) */ { D = memoryRead(MP) | 0x10; CONTEND_READ_NO_MREQ(MP, 1); memoryWrite(MP, D); };
        void _0xxxcbe3() /* LD E,SET 4,(REG+dd) */ { E = memoryRead(MP) | 0x10; CONTEND_READ_NO_MREQ(MP, 1); memoryWrite(MP, E); };
        void _0xxxcbe4() /* LD H,SET 4,(REG+dd) */ { H = memoryRead(MP) | 0x10; CONTEND_READ_NO_MREQ(MP, 1); memoryWrite(MP, H); };
        void _0xxxcbe5() /* LD L,SET 4,(REG+dd) */ { L = memoryRead(MP) | 0x10; CONTEND_READ_NO_MREQ(MP, 1); memoryWrite(MP, L); };
        void _0xxxcbe6() /* SET 4,(REG+dd)      */ { uint8_t t = memoryRead(MP); CONTEND_READ_NO_MREQ(MP, 1); memoryWrite(MP, t | 0x10); };
        void _0xxxcbe7() /* LD A,SET 4,(REG+dd) */ { A = memoryRead(MP) | 0x10; CONTEND_READ_NO_MREQ(MP, 1); memoryWrite(MP, A); };
        void _0xxxcbe8() /* LD B,SET 5,(REG+dd) */ { B = memoryRead(MP) | 0x20; CONTEND_READ_NO_MREQ(MP, 1); memoryWrite(MP, B); };
        void _0xxxcbe9() /* LD C,SET 5,(REG+dd) */ { C = memoryRead(MP) | 0x20; CONTEND_READ_NO_MREQ(MP, 1); memoryWrite(MP, C); };
        void _0xxxcbea() /* LD D,SET 5,(REG+dd) */ { D = memoryRead(MP) | 0x20; CONTEND_READ_NO_MREQ(MP, 1); memoryWrite(MP, D); };
        void _0xxxcbeb() /* LD E,SET 5,(REG+dd) */ { E = memoryRead(MP) | 0x20; CONTEND_READ_NO_MREQ(MP, 1); memoryWrite(MP, E); };
        void _0xxxcbec() /* LD H,SET 5,(REG+dd) */ { H = memoryRead(MP) | 0x20; CONTEND_READ_NO_MREQ(MP, 1); memoryWrite(MP, H); };
        void _0xxxcbed() /* LD L,SET 5,(REG+dd) */ { L = memoryRead(MP) | 0x20; CONTEND_READ_NO_MREQ(MP, 1); memoryWrite(MP, L); };
        void _0xxxcbee() /* SET 5,(REG+dd)      */ { uint8_t t = memoryRead(MP); CONTEND_READ_NO_MREQ(MP, 1); memoryWrite(MP, t | 0x20); };
        void _0xxxcbef() /* LD A,SET 5,(REG+dd) */ { A = memoryRead(MP) | 0x20; CONTEND_READ_NO_MREQ(MP, 1); memoryWrite(MP, A); };
        void _0xxxcbf0() /* LD B,SET 6,(REG+dd) */ { B = memoryRead(MP) | 0x40; CONTEND_READ_NO_MREQ(MP, 1); memoryWrite(MP, B); };
        void _0xxxcbf1() /* LD C,SET 6,(REG+dd) */ { C = memoryRead(MP) | 0x40; CONTEND_READ_NO_MREQ(MP, 1); memoryWrite(MP, C); };
        void _0xxxcbf2() /* LD D,SET 6,(REG+dd) */ { D = memoryRead(MP) | 0x40; CONTEND_READ_NO_MREQ(MP, 1); memoryWrite(MP, D); };
        void _0xxxcbf3() /* LD E,SET 6,(REG+dd) */ { E = memoryRead(MP) | 0x40; CONTEND_READ_NO_MREQ(MP, 1); memoryWrite(MP, E); };
        void _0xxxcbf4() /* LD H,SET 6,(REG+dd) */ { H = memoryRead(MP) | 0x40; CONTEND_READ_NO_MREQ(MP, 1); memoryWrite(MP, H); };
        void _0xxxcbf5() /* LD L,SET 6,(REG+dd) */ { L = memoryRead(MP) | 0x40; CONTEND_READ_NO_MREQ(MP, 1); memoryWrite(MP, L); };
        void _0xxxcbf6() /* SET 6,(REG+dd)      */ { uint8_t t = memoryRead(MP); CONTEND_READ_NO_MREQ(MP, 1); memoryWrite(MP, t | 0x40); };
        void _0xxxcbf7() /* LD A,SET 6,(REG+dd) */ { A = memoryRead(MP) | 0x40; CONTEND_READ_NO_MREQ(MP, 1); memoryWrite(MP, A); };
        void _0xxxcbf8() /* LD B,SET 7,(REG+dd) */ { B = memoryRead(MP) | 0x80; CONTEND_READ_NO_MREQ(MP, 1); memoryWrite(MP, B); };
        void _0xxxcbf9() /* LD C,SET 7,(REG+dd) */ { C = memoryRead(MP) | 0x80; CONTEND_READ_NO_MREQ(MP, 1); memoryWrite(MP, C); };
        void _0xxxcbfa() /* LD D,SET 7,(REG+dd) */ { D = memoryRead(MP) | 0x80; CONTEND_READ_NO_MREQ(MP, 1); memoryWrite(MP, D); };
        void _0xxxcbfb() /* LD E,SET 7,(REG+dd) */ { E = memoryRead(MP) | 0x80; CONTEND_READ_NO_MREQ(MP, 1); memoryWrite(MP, E); };
        void _0xxxcbfc() /* LD H,SET 7,(REG+dd) */ { H = memoryRead(MP) | 0x80; CONTEND_READ_NO_MREQ(MP, 1); memoryWrite(MP, H); };
        void _0xxxcbfd() /* LD L,SET 7,(REG+dd) */ { L = memoryRead(MP) | 0x80; CONTEND_READ_NO_MREQ(MP, 1); memoryWrite(MP, L); };
        void _0xxxcbfe() /* SET 7,(REG+dd)      */ { uint8_t t = memoryRead(MP); CONTEND_READ_NO_MREQ(MP, 1); memoryWrite(MP, t | 0x80); };
        void _0xxxcbff() /* LD A,SET 7,(REG+dd) */ { A = memoryRead(MP) | 0x80; CONTEND_READ_NO_MREQ(MP, 1); memoryWrite(MP, A); };

        void _0xed40() /* IN B,(C)      */ Z80_IN(B, BC);
        void _0xed41() /* OUT (C),B     */ { io::write(BC, B); MP = BC + 1; };
        void _0xed42() /* SBC HL,BC     */ { REP_7(CONTEND_READ_NO_MREQ(IR, 1)); SBCW(BC); };
        void _0xed43() /* LD (nnnn),BC  */ LDW_NNRR(C,B);
        void _0xed7c() /* NEG           */ { uint8_t v = A; A = 0; SUB(v); };
        void _0xed7d() /* RETN          */ { IFF1=IFF2; RET(); };
        void _0xed6e() /* IM 0          */ { IM = 0; };
        void _0xed47() /* LD I,A        */ { CONTEND_READ_NO_MREQ(IR, 1); I = A; };
        void _0xed48() /* IN C,(C)      */ Z80_IN(C, BC);
        void _0xed49() /* OUT (C),C     */ { io::write(BC, C); MP = BC + 1; };
        void _0xed4a() /* ADC HL,BC     */ { REP_7(CONTEND_READ_NO_MREQ(IR, 1)); ADCW(BC); };
        void _0xed4b() /* LD BC,(nnnn)  */ LDW_RRNN(C, B);
        void _0xed4f() /* LD R,A        */ { CONTEND_READ_NO_MREQ(IR, 1); R = R7 = A; };
        void _0xed50() /* IN D,(C)      */ Z80_IN( D, BC );
        void _0xed51() /* OUT (C),D     */ { io::write(BC, D); MP = BC + 1; };
        void _0xed52() /* SBC HL,DE     */ { REP_7(CONTEND_READ_NO_MREQ(IR, 1)); SBCW(DE); };
        void _0xed53() /* LD (nnnn),DE  */ LDW_NNRR(E, D);
        void _0xed76() /* IM 1          */ { IM = 1; };
        void _0xed57() /* LD A,I        */ { CONTEND_READ_NO_MREQ(IR, 1); A = I; F = (F & FLAG_C) | sz53[A] | (IFF2 ? FLAG_V : 0); Q = F; iff2Read = true; };
        void _0xed58() /* IN E,(C)      */ Z80_IN(E, BC);
        void _0xed59() /* OUT (C),E     */ { io::write(BC, E); MP = BC + 1; };
        void _0xed5a() /* ADC HL,DE     */ { REP_7(CONTEND_READ_NO_MREQ(IR, 1)); ADCW(DE); };
        void _0xed5b() /* LD DE,(nnnn)  */ LDW_RRNN(E, D);
        void _0xed7e() /* IM 2          */ { IM = 2; };
        void _0xed5f() /* LD A,R        */ { CONTEND_READ_NO_MREQ(IR, 1); A = (R & 0x7f) | (R7 & 0x80); F = (F & FLAG_C) | sz53[A] | (IFF2 ? FLAG_V : 0); Q = F; iff2Read = true; };
        void _0xed60() /* IN H,(C)      */ Z80_IN(H, BC);
        void _0xed61() /* OUT (C),H     */ { io::write(BC, H); MP = BC + 1; };
        void _0xed62() /* SBC HL,HL     */ { REP_7(CONTEND_READ_NO_MREQ(IR, 1)); SBCW(HL); };
        void _0xed63() /* LD (nnnn),HL  */ LDW_NNRR(L, H);
        void _0xed67() /* RRD           */ { uint8_t v = memoryRead(HL); REP_4(CONTEND_READ_NO_MREQ(HL, 1)); memoryWrite(HL, (A << 4) | (v >> 4)); A = (A & 0xf0) | (v & 0x0f); F = (F & FLAG_C) | sz53p[A]; Q = F; MP = HL + 1; };
        void _0xed68() /* IN L,(C)      */ Z80_IN(L, BC);
        void _0xed69() /* OUT (C),L     */ { io::write(BC, L); MP = BC + 1; };
        void _0xed6a() /* ADC HL,HL     */ { REP_7(CONTEND_READ_NO_MREQ(IR, 1)); ADCW(HL); };
        void _0xed6b() /* LD HL,(nnnn)  */ LDW_RRNN(L, H);
        void _0xed6f() /* RLD           */ { uint8_t v = memoryRead(HL); REP_4(CONTEND_READ_NO_MREQ(HL, 1)); memoryWrite(HL, (v << 4) | (A & 0x0f)); A = (A & 0xf0) | (v >> 4 ); F = (F & FLAG_C) | sz53p[A]; Q = F; MP = HL + 1; };
        void _0xed70() /* IN F,(C)      */ Z80_IN(F, BC);
        void _0xed71() /* OUT (C),0     */ { io::write(BC, IS_CMOS ? 0xff : 0); MP = BC + 1; };
        void _0xed72() /* SBC HL,SP     */ { REP_7(CONTEND_READ_NO_MREQ(IR, 1)); SBCW(SP); };
        void _0xed73() /* LD (nnnn),SP  */ LDW_NNRR(SPL, SPH);
        void _0xed78() /* IN A,(C)      */ Z80_IN(A, BC);
        void _0xed79() /* OUT (C),A     */ { io::write(BC, A); MP = BC + 1; };
        void _0xed7a() /* ADC HL,SP     */ { REP_7(CONTEND_READ_NO_MREQ(IR, 1)); ADCW(SP); };
        void _0xed7b() /* LD SP,(nnnn)  */ LDW_RRNN(SPL, SPH);
        void _0xeda0() /* LDI           */ { uint8_t v = memoryRead(HL); BC--; memoryWrite(DE, v); REP_2(CONTEND_WRITE_NO_MREQ(DE, 1)); ++DE; ++HL; v += A; F = (F & (FLAG_C | FLAG_Z | FLAG_S)) | (BC ? FLAG_V : 0) | (v & FLAG_3) | ((v & 0x02) ? FLAG_5 : 0); Q = F; };;
        void _0xeda1() /* CPI           */ { uint8_t v = memoryRead(HL), t = A - v, l = ((A & 0x08) >> 3) | ((v & 0x08) >> 2) | ((t & 0x08) >> 1); REP_5(CONTEND_READ_NO_MREQ(HL, 1)); ++HL; --BC; F = (F & FLAG_C) | (BC ? (FLAG_V | FLAG_N) : FLAG_N) | halfcarrySub[l] | (t ? 0 : FLAG_Z) | (t & FLAG_S); if (F & FLAG_H) t--; F |= (t & FLAG_3) | ((t & 0x02) ? FLAG_5 : 0); Q = F; ++MP; };
        void _0xeda2() /* INI           */ { uint8_t t1, t2; CONTEND_READ_NO_MREQ(IR, 1); t1 = io::read(BC); memoryWrite(HL, t1); MP = BC + 1; --B; ++HL; t2 = t1 + C + 1; F = ((t1 & 0x80) ? FLAG_N : 0) | ((t2 < t1) ? FLAG_H | FLAG_C : 0) | ( parity[(t2 & 0x07) ^ B] ? FLAG_P : 0) | sz53[B]; Q = F; };
        void _0xeda3() /* OUTI          */ { uint8_t t1, t2; CONTEND_READ_NO_MREQ(IR, 1); t1 = memoryRead(HL); --B; MP = BC + 1; io::write(BC, t1); ++HL; t2 = t1 + L; F = ((t1 & 0x80) ? FLAG_N : 0) |((t2 < t1) ? FLAG_H | FLAG_C : 0) | (parity[(t2 & 0x07) ^ B] ? FLAG_P : 0) | sz53[B]; Q = F; };
        void _0xeda8() /* LDD           */ { uint8_t t = memoryRead(HL); --BC; memoryWrite(DE, t); REP_2(CONTEND_WRITE_NO_MREQ(DE, 1)); --DE; --HL; t += A; F = (F & (FLAG_C | FLAG_Z | FLAG_S)) | (BC ? FLAG_V : 0) | (t & FLAG_3) | ((t & 0x02) ? FLAG_5 : 0); Q = F; };
        void _0xeda9() /* CPD           */ { uint8_t v = memoryRead(HL), t = A - v, l = ((A & 0x08) >> 3) | ((v & 0x08) >> 2 ) | ((t & 0x08) >> 1); REP_5(CONTEND_READ_NO_MREQ(HL, 1)); --HL; --BC; F = (F & FLAG_C) | (BC ? (FLAG_V | FLAG_N) : FLAG_N) | halfcarrySub[l] | (t ? 0 : FLAG_Z) | (t & FLAG_S); if (F & FLAG_H) --t; F |= (t & FLAG_3) | ((t & 0x02) ? FLAG_5 : 0); Q = F; --MP; };
        void _0xedaa() /* IND           */ { uint8_t t1, t2; CONTEND_READ_NO_MREQ(IR, 1); t1 = io::read(BC); memoryWrite(HL, t1); MP = BC - 1; --B; --HL; t2 = t1 + C - 1; F = ((t1 & 0x80) ? FLAG_N : 0) | ((t2 < t1) ? FLAG_H | FLAG_C : 0) | (parity[(t2 & 0x07) ^ B] ? FLAG_P : 0) | sz53[B]; Q = F; };
        void _0xedab() /* OUTD          */ { uint8_t t1, t2; CONTEND_READ_NO_MREQ(IR, 1); t1 = memoryRead(HL); --B; MP = BC - 1; io::write(BC, t1); --HL; t2 = t1 + L; F = ((t1 & 0x80) ? FLAG_N : 0) | ((t2 < t1) ? FLAG_H | FLAG_C : 0) | (parity[(t2 & 0x07) ^ B] ? FLAG_P : 0) | sz53[B]; Q = F; };
        void _0xedb0() /* LDIR          */ { uint8_t t = memoryRead(HL); memoryWrite(DE, t); REP_2(CONTEND_WRITE_NO_MREQ(DE, 1)); --BC; t += A; F = (F & (FLAG_C | FLAG_Z | FLAG_S)) | (BC ? FLAG_V : 0) | (t & FLAG_3) | ((t & 0x02) ? FLAG_5 : 0); Q = F; if (BC) { REP_5(CONTEND_WRITE_NO_MREQ(DE, 1)); PC -= 2; MP = PC + 1; } ++HL; ++DE; };
        void _0xedb1() /* CPIR          */ { uint8_t v = memoryRead(HL), t = A - v, l = ((A & 0x08) >> 3) | ((v & 0x08) >> 2) | ((t & 0x08) >> 1); REP_5(CONTEND_READ_NO_MREQ(HL, 1)); --BC; F = (F & FLAG_C) | (BC ? (FLAG_V | FLAG_N) : FLAG_N) | halfcarrySub[l] | (t ? 0 : FLAG_Z) | (t & FLAG_S); if (F & FLAG_H) --t; F |= (t & FLAG_3) | ((t & 0x02) ? FLAG_5 : 0); Q = F; if ((F & (FLAG_V | FLAG_Z)) == FLAG_V) { REP_5(CONTEND_READ_NO_MREQ(HL, 1)); PC -= 2; MP = PC + 1; } else { ++MP; } ++HL; };
        void _0xedb2() /* INIR          */ { uint8_t t1, t2; CONTEND_READ_NO_MREQ(IR, 1); t1 =io::read(BC); memoryWrite(HL, t1); MP = BC + 1; --B; t2 = t1 + C + 1; F = ((t1 & 0x80) ? FLAG_N : 0) | ((t2 < t1) ? FLAG_H | FLAG_C : 0) | (parity[(t2 & 0x07) ^ B] ? FLAG_P : 0) | sz53[B]; Q = F; if (B) { REP_5(CONTEND_WRITE_NO_MREQ(HL, 1)); PC -= 2; } ++HL; };
        void _0xedb3() /* OTIR          */ { uint8_t t1, t2; CONTEND_READ_NO_MREQ(IR, 1); t1 = memoryRead(HL); --B; MP = BC + 1; io::write(BC, t1); ++HL; t2 = t1 + L; F = ((t1 & 0x80) ? FLAG_N : 0) | ((t2 < t1) ? FLAG_H | FLAG_C : 0) | (parity[(t2 & 0x07) ^ B] ? FLAG_P : 0) | sz53[B]; Q = F; if(B) { REP_5(CONTEND_READ_NO_MREQ(BC, 1)); PC -= 2; } };
        void _0xedb8() /* LDDR          */ { uint8_t b = memoryRead(HL); memoryWrite(DE, b); REP_2(CONTEND_WRITE_NO_MREQ(DE, 1)); --BC; b += A; F = (F & (FLAG_C | FLAG_Z | FLAG_S)) | (BC ? FLAG_V : 0) | (b & FLAG_3) | ((b & 0x02) ? FLAG_5 : 0); Q = F; if (BC) { REP_5(CONTEND_WRITE_NO_MREQ(DE, 1)); PC -= 2; MP = PC + 1; } --HL; --DE; };
        void _0xedb9() /* CPDR          */ { uint8_t v = memoryRead(HL), b = A - v, l = ((A & 0x08) >> 3) | ((v & 0x08) >> 2) | ((b & 0x08) >> 1); REP_5(CONTEND_READ_NO_MREQ(HL, 1)); --BC; F = (F & FLAG_C) | (BC ? (FLAG_V | FLAG_N) : FLAG_N) | halfcarrySub[l] | (b ? 0 : FLAG_Z) | (b & FLAG_S); if (F & FLAG_H) --b; F |= (b & FLAG_3) | ((b & 0x02) ? FLAG_5 : 0); Q = F; if ((F & (FLAG_V | FLAG_Z)) == FLAG_V) { REP_5(CONTEND_READ_NO_MREQ(HL, 1)); PC -= 2; MP = PC + 1; } else { --MP; } --HL; };
        void _0xedba() /* INDR          */ { uint8_t t1, t2; CONTEND_READ_NO_MREQ(IR, 1); t1 = io::read(BC); memoryWrite(HL, t1); MP = BC - 1; --B; t2 = t1 + C - 1; F = ((t1 & 0x80) ? FLAG_N : 0) | ((t2 < t1) ? FLAG_H | FLAG_C : 0) | (parity[(t2 & 0x07) ^ B] ? FLAG_P : 0) | sz53[B]; Q = F; if (B) { REP_5(CONTEND_WRITE_NO_MREQ(HL, 1)); PC -= 2; } --HL; };
        void _0xedbb() /* OTDR          */ { uint8_t t1, t2; CONTEND_READ_NO_MREQ(IR, 1); t1 = memoryRead(HL); --B; MP = BC - 1; io::write(BC, t1); --HL; t2 = t1 + L; F = ((t1 & 0x80) ? FLAG_N : 0) | ((t2 < t1) ? FLAG_H | FLAG_C : 0) | (parity[(t2 & 0x07) ^ B] ? FLAG_P : 0) | sz53[B]; Q = F; if (B) { REP_5(CONTEND_READ_NO_MREQ(BC, 1)); PC -= 2; } };

        void(*opCodes[0x100])() =
        {
            _NOP_, _0x01, _0x02, _0x03, _0x04, _0x05, _0x06, _0x07, _0x08, _0x09, _0x0a, _0x0b, _0x0c, _0x0d, _0x0e, _0x0f,
            _0x10, _0x11, _0x12, _0x13, _0x14, _0x15, _0x16, _0x17, _0x18, _0x19, _0x1a, _0x1b, _0x1c, _0x1d, _0x1e, _0x1f,
            _0x20, _0x21, _0x22, _0x23, _0x24, _0x25, _0x26, _0x27, _0x28, _0x29, _0x2a, _0x2b, _0x2c, _0x2d, _0x2e, _0x2f,
            _0x30, _0x31, _0x32, _0x33, _0x34, _0x35, _0x36, _0x37, _0x38, _0x39, _0x3a, _0x3b, _0x3c, _0x3d, _0x3e, _0x3f,
            _NOP_, _0x41, _0x42, _0x43, _0x44, _0x45, _0x46, _0x47, _0x48, _NOP_, _0x4a, _0x4b, _0x4c, _0x4d, _0x4e, _0x4f,
            _0x50, _0x51, _NOP_, _0x53, _0x54, _0x55, _0x56, _0x57, _0x58, _0x59, _0x5a, _NOP_, _0x5c, _0x5d, _0x5e, _0x5f,
            _0x60, _0x61, _0x62, _0x63, _NOP_, _0x65, _0x66, _0x67, _0x68, _0x69, _0x6a, _0x6b, _0x6c, _NOP_, _0x6e, _0x6f,
            _0x70, _0x71, _0x72, _0x73, _0x74, _0x75, _0x76, _0x77, _0x78, _0x79, _0x7a, _0x7b, _0x7c, _0x7d, _0x7e, _NOP_,
            _0x80, _0x81, _0x82, _0x83, _0x84, _0x85, _0x86, _0x87, _0x88, _0x89, _0x8a, _0x8b, _0x8c, _0x8d, _0x8e, _0x8f,
            _0x90, _0x91, _0x92, _0x93, _0x94, _0x95, _0x96, _0x97, _0x98, _0x99, _0x9a, _0x9b, _0x9c, _0x9d, _0x9e, _0x9f,
            _0xa0, _0xa1, _0xa2, _0xa3, _0xa4, _0xa5, _0xa6, _0xa7, _0xa8, _0xa9, _0xaa, _0xab, _0xac, _0xad, _0xae, _0xaf,
            _0xb0, _0xb1, _0xb2, _0xb3, _0xb4, _0xb5, _0xb6, _0xb7, _0xb8, _0xb9, _0xba, _0xbb, _0xbc, _0xbd, _0xbe, _0xbf,
            _0xc0, _0xc1, _0xc2, _0xc3, _0xc4, _0xc5, _0xc6, _0xc7, _0xc8, _0xc9, _0xca, _0xcb, _0xcc, _0xcd, _0xce, _0xcf,
            _0xd0, _0xd1, _0xd2, _0xd3, _0xd4, _0xd5, _0xd6, _0xd7, _0xd8, _0xd9, _0xda, _0xdb, _0xdc, _0xdd, _0xde, _0xdf,
            _0xe0, _0xe1, _0xe2, _0xe3, _0xe4, _0xe5, _0xe6, _0xe7, _0xe8, _0xe9, _0xea, _0xeb, _0xec, _0xed, _0xee, _0xef,
            _0xf0, _0xf1, _0xf2, _0xf3, _0xf4, _0xf5, _0xf6, _0xf7, _0xf8, _0xf9, _0xfa, _0xfb, _0xfc, _0xfd, _0xfe, _0xff
        };

        void(*opCodesCB[0x100])() =
        {
            _0xcb00, _0xcb01, _0xcb02, _0xcb03, _0xcb04, _0xcb05, _0xcb06, _0xcb07, _0xcb08, _0xcb09, _0xcb0a, _0xcb0b, _0xcb0c, _0xcb0d, _0xcb0e, _0xcb0f,
            _0xcb10, _0xcb11, _0xcb12, _0xcb13, _0xcb14, _0xcb15, _0xcb16, _0xcb17, _0xcb18, _0xcb19, _0xcb1a, _0xcb1b, _0xcb1c, _0xcb1d, _0xcb1e, _0xcb1f,
            _0xcb20, _0xcb21, _0xcb22, _0xcb23, _0xcb24, _0xcb25, _0xcb26, _0xcb27, _0xcb28, _0xcb29, _0xcb2a, _0xcb2b, _0xcb2c, _0xcb2d, _0xcb2e, _0xcb2f,
            _0xcb30, _0xcb31, _0xcb32, _0xcb33, _0xcb34, _0xcb35, _0xcb36, _0xcb37, _0xcb38, _0xcb39, _0xcb3a, _0xcb3b, _0xcb3c, _0xcb3d, _0xcb3e, _0xcb3f,
            _0xcb40, _0xcb41, _0xcb42, _0xcb43, _0xcb44, _0xcb45, _0xcb46, _0xcb47, _0xcb48, _0xcb49, _0xcb4a, _0xcb4b, _0xcb4c, _0xcb4d, _0xcb4e, _0xcb4f,
            _0xcb50, _0xcb51, _0xcb52, _0xcb53, _0xcb54, _0xcb55, _0xcb56, _0xcb57, _0xcb58, _0xcb59, _0xcb5a, _0xcb5b, _0xcb5c, _0xcb5d, _0xcb5e, _0xcb5f,
            _0xcb60, _0xcb61, _0xcb62, _0xcb63, _0xcb64, _0xcb65, _0xcb66, _0xcb67, _0xcb68, _0xcb69, _0xcb6a, _0xcb6b, _0xcb6c, _0xcb6d, _0xcb6e, _0xcb6f,
            _0xcb70, _0xcb71, _0xcb72, _0xcb73, _0xcb74, _0xcb75, _0xcb76, _0xcb77, _0xcb78, _0xcb79, _0xcb7a, _0xcb7b, _0xcb7c, _0xcb7d, _0xcb7e, _0xcb7f,
            _0xcb80, _0xcb81, _0xcb82, _0xcb83, _0xcb84, _0xcb85, _0xcb86, _0xcb87, _0xcb88, _0xcb89, _0xcb8a, _0xcb8b, _0xcb8c, _0xcb8d, _0xcb8e, _0xcb8f,
            _0xcb90, _0xcb91, _0xcb92, _0xcb93, _0xcb94, _0xcb95, _0xcb96, _0xcb97, _0xcb98, _0xcb99, _0xcb9a, _0xcb9b, _0xcb9c, _0xcb9d, _0xcb9e, _0xcb9f,
            _0xcba0, _0xcba1, _0xcba2, _0xcba3, _0xcba4, _0xcba5, _0xcba6, _0xcba7, _0xcba8, _0xcba9, _0xcbaa, _0xcbab, _0xcbac, _0xcbad, _0xcbae, _0xcbaf,
            _0xcbb0, _0xcbb1, _0xcbb2, _0xcbb3, _0xcbb4, _0xcbb5, _0xcbb6, _0xcbb7, _0xcbb8, _0xcbb9, _0xcbba, _0xcbbb, _0xcbbc, _0xcbbd, _0xcbbe, _0xcbbf,
            _0xcbc0, _0xcbc1, _0xcbc2, _0xcbc3, _0xcbc4, _0xcbc5, _0xcbc6, _0xcbc7, _0xcbc8, _0xcbc9, _0xcbca, _0xcbcb, _0xcbcc, _0xcbcd, _0xcbce, _0xcbcf,
            _0xcbd0, _0xcbd1, _0xcbd2, _0xcbd3, _0xcbd4, _0xcbd5, _0xcbd6, _0xcbd7, _0xcbd8, _0xcbd9, _0xcbda, _0xcbdb, _0xcbdc, _0xcbdd, _0xcbde, _0xcbdf,
            _0xcbe0, _0xcbe1, _0xcbe2, _0xcbe3, _0xcbe4, _0xcbe5, _0xcbe6, _0xcbe7, _0xcbe8, _0xcbe9, _0xcbea, _0xcbeb, _0xcbec, _0xcbed, _0xcbee, _0xcbef,
            _0xcbf0, _0xcbf1, _0xcbf2, _0xcbf3, _0xcbf4, _0xcbf5, _0xcbf6, _0xcbf7, _0xcbf8, _0xcbf9, _0xcbfa, _0xcbfb, _0xcbfc, _0xcbfd, _0xcbfe, _0xcbff
        };

        void(*opCodesDD[0x100])() =
        {
            _RETRY_, _RETRY_, _RETRY_, _RETRY_, _RETRY_, _RETRY_, _RETRY_, _RETRY_, _RETRY_, _0xdd09, _RETRY_, _RETRY_, _RETRY_, _RETRY_, _RETRY_, _RETRY_,
            _RETRY_, _RETRY_, _RETRY_, _RETRY_, _RETRY_, _RETRY_, _RETRY_, _RETRY_, _RETRY_, _0xdd19, _RETRY_, _RETRY_, _RETRY_, _RETRY_, _RETRY_, _RETRY_,
            _RETRY_, _0xdd21, _0xdd22, _0xdd23, _0xdd24, _0xdd25, _0xdd26, _RETRY_, _RETRY_, _0xdd29, _0xdd2a, _0xdd2b, _0xdd2c, _0xdd2d, _0xdd2e, _RETRY_,
            _RETRY_, _RETRY_, _RETRY_, _RETRY_, _0xdd34, _0xdd35, _0xdd36, _RETRY_, _RETRY_, _0xdd39, _RETRY_, _RETRY_, _RETRY_, _RETRY_, _RETRY_, _RETRY_,
            _RETRY_, _RETRY_, _RETRY_, _RETRY_, _0xdd44, _0xdd45, _0xdd46, _RETRY_, _RETRY_, _RETRY_, _RETRY_, _RETRY_, _0xdd4c, _0xdd4d, _0xdd4e, _RETRY_,
            _RETRY_, _RETRY_, _RETRY_, _RETRY_, _0xdd54, _0xdd55, _0xdd56, _RETRY_, _RETRY_, _RETRY_, _RETRY_, _RETRY_, _0xdd5c, _0xdd5d, _0xdd5e, _RETRY_,
            _0xdd60, _0xdd61, _0xdd62, _0xdd63,   _NOP_, _0xdd65, _0xdd66, _0xdd67, _0xdd68, _0xdd69, _0xdd6a, _0xdd6b, _0xdd6c,   _NOP_, _0xdd6e, _0xdd6f,
            _0xdd70, _0xdd71, _0xdd72, _0xdd73, _0xdd74, _0xdd75, _RETRY_, _0xdd77, _RETRY_, _RETRY_, _RETRY_, _RETRY_, _0xdd7c, _0xdd7d, _0xdd7e, _RETRY_,
            _RETRY_, _RETRY_, _RETRY_, _RETRY_, _0xdd84, _0xdd85, _0xdd86, _RETRY_, _RETRY_, _RETRY_, _RETRY_, _RETRY_, _0xdd8c, _0xdd8d, _0xdd8e, _RETRY_,
            _RETRY_, _RETRY_, _RETRY_, _RETRY_, _0xdd94, _0xdd95, _0xdd96, _RETRY_, _RETRY_, _RETRY_, _RETRY_, _RETRY_, _0xdd9c, _0xdd9d, _0xdd9e, _RETRY_,
            _RETRY_, _RETRY_, _RETRY_, _RETRY_, _0xdda4, _0xdda5, _0xdda6, _RETRY_, _RETRY_, _RETRY_, _RETRY_, _RETRY_, _0xddac, _0xddad, _0xddae, _RETRY_,
            _RETRY_, _RETRY_, _RETRY_, _RETRY_, _0xddb4, _0xddb5, _0xddb6, _RETRY_, _RETRY_, _RETRY_, _RETRY_, _RETRY_, _0xddbc, _0xddbd, _0xddbe, _RETRY_,
            _RETRY_, _RETRY_, _RETRY_, _RETRY_, _RETRY_, _RETRY_, _RETRY_, _RETRY_, _RETRY_, _RETRY_, _RETRY_, _0xddcb, _RETRY_, _RETRY_, _RETRY_, _RETRY_,
            _RETRY_, _RETRY_, _RETRY_, _RETRY_, _RETRY_, _RETRY_, _RETRY_, _RETRY_, _RETRY_, _RETRY_, _RETRY_, _RETRY_, _RETRY_, _RETRY_, _RETRY_, _RETRY_,
            _RETRY_, _0xdde1, _RETRY_, _0xdde3, _RETRY_, _0xdde5, _RETRY_, _RETRY_, _RETRY_, _0xdde9, _RETRY_, _RETRY_, _RETRY_, _RETRY_, _RETRY_, _RETRY_,
            _RETRY_, _RETRY_, _RETRY_, _RETRY_, _RETRY_, _RETRY_, _RETRY_, _RETRY_, _RETRY_, _0xddf9, _RETRY_, _RETRY_, _RETRY_, _RETRY_, _RETRY_, _RETRY_
        };

        void(*opCodesFD[0x100])() =
        {
            _RETRY_, _RETRY_, _RETRY_, _RETRY_, _RETRY_, _RETRY_, _RETRY_, _RETRY_, _RETRY_, _0xfd09, _RETRY_, _RETRY_, _RETRY_, _RETRY_, _RETRY_, _RETRY_,
            _RETRY_, _RETRY_, _RETRY_, _RETRY_, _RETRY_, _RETRY_, _RETRY_, _RETRY_, _RETRY_, _0xfd19, _RETRY_, _RETRY_, _RETRY_, _RETRY_, _RETRY_, _RETRY_,
            _RETRY_, _0xfd21, _0xfd22, _0xfd23, _0xfd24, _0xfd25, _0xfd26, _RETRY_, _RETRY_, _0xfd29, _0xfd2a, _0xfd2b, _0xfd2c, _0xfd2d, _0xfd2e, _RETRY_,
            _RETRY_, _RETRY_, _RETRY_, _RETRY_, _0xfd34, _0xfd35, _0xfd36, _RETRY_, _RETRY_, _0xfd39, _RETRY_, _RETRY_, _RETRY_, _RETRY_, _RETRY_, _RETRY_,
            _RETRY_, _RETRY_, _RETRY_, _RETRY_, _0xfd44, _0xfd45, _0xfd46, _RETRY_, _RETRY_, _RETRY_, _RETRY_, _RETRY_, _0xfd4c, _0xfd4d, _0xfd4e, _RETRY_,
            _RETRY_, _RETRY_, _RETRY_, _RETRY_, _0xfd54, _0xfd55, _0xfd56, _RETRY_, _RETRY_, _RETRY_, _RETRY_, _RETRY_, _0xfd5c, _0xfd5d, _0xfd5e, _RETRY_,
            _0xfd60, _0xfd61, _0xfd62, _0xfd63,   _NOP_, _0xfd65, _0xfd66, _0xfd67, _0xfd68, _0xfd69, _0xfd6a, _0xfd6b, _0xfd6c,   _NOP_, _0xfd6e, _0xfd6f,
            _0xfd70, _0xfd71, _0xfd72, _0xfd73, _0xfd74, _0xfd75, _RETRY_, _0xfd77, _RETRY_, _RETRY_, _RETRY_, _RETRY_, _0xfd7c, _0xfd7d, _0xfd7e, _RETRY_,
            _RETRY_, _RETRY_, _RETRY_, _RETRY_, _0xfd84, _0xfd85, _0xfd86, _RETRY_, _RETRY_, _RETRY_, _RETRY_, _RETRY_, _0xfd8c, _0xfd8d, _0xfd8e, _RETRY_,
            _RETRY_, _RETRY_, _RETRY_, _RETRY_, _0xfd94, _0xfd95, _0xfd96, _RETRY_, _RETRY_, _RETRY_, _RETRY_, _RETRY_, _0xfd9c, _0xfd9d, _0xfd9e, _RETRY_,
            _RETRY_, _RETRY_, _RETRY_, _RETRY_, _0xfda4, _0xfda5, _0xfda6, _RETRY_, _RETRY_, _RETRY_, _RETRY_, _RETRY_, _0xfdac, _0xfdad, _0xfdae, _RETRY_,
            _RETRY_, _RETRY_, _RETRY_, _RETRY_, _0xfdb4, _0xfdb5, _0xfdb6, _RETRY_, _RETRY_, _RETRY_, _RETRY_, _RETRY_, _0xfdbc, _0xfdbd, _0xfdbe, _RETRY_,
            _RETRY_, _RETRY_, _RETRY_, _RETRY_, _RETRY_, _RETRY_, _RETRY_, _RETRY_, _RETRY_, _RETRY_, _RETRY_, _0xfdcb, _RETRY_, _RETRY_, _RETRY_, _RETRY_,
            _RETRY_, _RETRY_, _RETRY_, _RETRY_, _RETRY_, _RETRY_, _RETRY_, _RETRY_, _RETRY_, _RETRY_, _RETRY_, _RETRY_, _RETRY_, _RETRY_, _RETRY_, _RETRY_,
            _RETRY_, _0xfde1, _RETRY_, _0xfde3, _RETRY_, _0xfde5, _RETRY_, _RETRY_, _RETRY_, _0xfde9, _RETRY_, _RETRY_, _RETRY_, _RETRY_, _RETRY_, _RETRY_,
            _RETRY_, _RETRY_, _RETRY_, _RETRY_, _RETRY_, _RETRY_, _RETRY_, _RETRY_, _RETRY_, _0xfdf9, _RETRY_, _RETRY_, _RETRY_, _RETRY_, _RETRY_, _RETRY_
        };

        void(*opCodesED[0x100])() =
        {
              _NOP_,   _NOP_,   _NOP_,   _NOP_,   _NOP_,   _NOP_,   _NOP_,   _NOP_,   _NOP_,   _NOP_,   _NOP_,   _NOP_,   _NOP_,   _NOP_,   _NOP_,   _NOP_,
              _NOP_,   _NOP_,   _NOP_,   _NOP_,   _NOP_,   _NOP_,   _NOP_,   _NOP_,   _NOP_,   _NOP_,   _NOP_,   _NOP_,   _NOP_,   _NOP_,   _NOP_,   _NOP_,
              _NOP_,   _NOP_,   _NOP_,   _NOP_,   _NOP_,   _NOP_,   _NOP_,   _NOP_,   _NOP_,   _NOP_,   _NOP_,   _NOP_,   _NOP_,   _NOP_,   _NOP_,   _NOP_,
              _NOP_,   _NOP_,   _NOP_,   _NOP_,   _NOP_,   _NOP_,   _NOP_,   _NOP_,   _NOP_,   _NOP_,   _NOP_,   _NOP_,   _NOP_,   _NOP_,   _NOP_,   _NOP_,
            _0xed40, _0xed41, _0xed42, _0xed43, _0xed7c, _0xed7d, _0xed6e, _0xed47, _0xed48, _0xed49, _0xed4a, _0xed4b, _0xed7c, _0xed7d, _0xed6e, _0xed4f,
            _0xed50, _0xed51, _0xed52, _0xed53, _0xed7c, _0xed7d, _0xed76, _0xed57, _0xed58, _0xed59, _0xed5a, _0xed5b, _0xed7c, _0xed7d, _0xed7e, _0xed5f,
            _0xed60, _0xed61, _0xed62, _0xed63, _0xed7c, _0xed7d, _0xed6e, _0xed67, _0xed68, _0xed69, _0xed6a, _0xed6b, _0xed7c, _0xed7d, _0xed6e, _0xed6f,
            _0xed70, _0xed71, _0xed72, _0xed73, _0xed7c, _0xed7d, _0xed76,   _NOP_, _0xed78, _0xed79, _0xed7a, _0xed7b, _0xed7c, _0xed7d, _0xed7e,   _NOP_,
              _NOP_,   _NOP_,   _NOP_,   _NOP_,   _NOP_,   _NOP_,   _NOP_,   _NOP_,   _NOP_,   _NOP_,   _NOP_,   _NOP_,   _NOP_,   _NOP_,   _NOP_,   _NOP_,
              _NOP_,   _NOP_,   _NOP_,   _NOP_,   _NOP_,   _NOP_,   _NOP_,   _NOP_,   _NOP_,   _NOP_,   _NOP_,   _NOP_,   _NOP_,   _NOP_,   _NOP_,   _NOP_,
            _0xeda0, _0xeda1, _0xeda2, _0xeda3,   _NOP_,   _NOP_,   _NOP_,   _NOP_, _0xeda8, _0xeda9, _0xedaa, _0xedab,   _NOP_,   _NOP_,   _NOP_,   _NOP_,
            _0xedb0, _0xedb1, _0xedb2, _0xedb3,   _NOP_,   _NOP_,   _NOP_,   _NOP_, _0xedb8, _0xedb9, _0xedba, _0xedbb,   _NOP_,   _NOP_,   _NOP_,   _NOP_,
              _NOP_,   _NOP_,   _NOP_,   _NOP_,   _NOP_,   _NOP_,   _NOP_,   _NOP_,   _NOP_,   _NOP_,   _NOP_,   _NOP_,   _NOP_,   _NOP_,   _NOP_,   _NOP_,
              _NOP_,   _NOP_,   _NOP_,   _NOP_,   _NOP_,   _NOP_,   _NOP_,   _NOP_,   _NOP_,   _NOP_,   _NOP_,   _NOP_,   _NOP_,   _NOP_,   _NOP_,   _NOP_,
              _NOP_,   _NOP_,   _NOP_,   _NOP_,   _NOP_,   _NOP_,   _NOP_,   _NOP_,   _NOP_,   _NOP_,   _NOP_,   _NOP_,   _NOP_,   _NOP_,   _NOP_,   _NOP_,
              _NOP_,   _NOP_,   _NOP_,   _NOP_,   _NOP_,   _NOP_,   _NOP_,   _NOP_,   _NOP_,   _NOP_,   _NOP_,   _NOP_,   _NOP_,   _NOP_,   _NOP_,   _NOP_
        };

        void(*opCodesXXCB[0x100])() =
        {
            _0xxxcb00, _0xxxcb01, _0xxxcb02, _0xxxcb03, _0xxxcb04, _0xxxcb05, _0xxxcb06, _0xxxcb07, _0xxxcb08, _0xxxcb09, _0xxxcb0a, _0xxxcb0b, _0xxxcb0c, _0xxxcb0d, _0xxxcb0e, _0xxxcb0f,
            _0xxxcb10, _0xxxcb11, _0xxxcb12, _0xxxcb13, _0xxxcb14, _0xxxcb15, _0xxxcb16, _0xxxcb17, _0xxxcb18, _0xxxcb19, _0xxxcb1a, _0xxxcb1b, _0xxxcb1c, _0xxxcb1d, _0xxxcb1e, _0xxxcb1f,
            _0xxxcb20, _0xxxcb21, _0xxxcb22, _0xxxcb23, _0xxxcb24, _0xxxcb25, _0xxxcb26, _0xxxcb27, _0xxxcb28, _0xxxcb29, _0xxxcb2a, _0xxxcb2b, _0xxxcb2c, _0xxxcb2d, _0xxxcb2e, _0xxxcb2f,
            _0xxxcb30, _0xxxcb31, _0xxxcb32, _0xxxcb33, _0xxxcb34, _0xxxcb35, _0xxxcb36, _0xxxcb37, _0xxxcb38, _0xxxcb39, _0xxxcb3a, _0xxxcb3b, _0xxxcb3c, _0xxxcb3d, _0xxxcb3e, _0xxxcb3f,
            _0xxxcb47, _0xxxcb47, _0xxxcb47, _0xxxcb47, _0xxxcb47, _0xxxcb47, _0xxxcb47, _0xxxcb47, _0xxxcb4f, _0xxxcb4f, _0xxxcb4f, _0xxxcb4f, _0xxxcb4f, _0xxxcb4f, _0xxxcb4f, _0xxxcb4f,
            _0xxxcb57, _0xxxcb57, _0xxxcb57, _0xxxcb57, _0xxxcb57, _0xxxcb57, _0xxxcb57, _0xxxcb57, _0xxxcb5f, _0xxxcb5f, _0xxxcb5f, _0xxxcb5f, _0xxxcb5f, _0xxxcb5f, _0xxxcb5f, _0xxxcb5f,
            _0xxxcb67, _0xxxcb67, _0xxxcb67, _0xxxcb67, _0xxxcb67, _0xxxcb67, _0xxxcb67, _0xxxcb67, _0xxxcb6f, _0xxxcb6f, _0xxxcb6f, _0xxxcb6f, _0xxxcb6f, _0xxxcb6f, _0xxxcb6f, _0xxxcb6f,
            _0xxxcb77, _0xxxcb77, _0xxxcb77, _0xxxcb77, _0xxxcb77, _0xxxcb77, _0xxxcb77, _0xxxcb77, _0xxxcb7f, _0xxxcb7f, _0xxxcb7f, _0xxxcb7f, _0xxxcb7f, _0xxxcb7f, _0xxxcb7f, _0xxxcb7f,
            _0xxxcb80, _0xxxcb81, _0xxxcb82, _0xxxcb83, _0xxxcb84, _0xxxcb85, _0xxxcb86, _0xxxcb87, _0xxxcb88, _0xxxcb89, _0xxxcb8a, _0xxxcb8b, _0xxxcb8c, _0xxxcb8d, _0xxxcb8e, _0xxxcb8f,
            _0xxxcb90, _0xxxcb91, _0xxxcb92, _0xxxcb93, _0xxxcb94, _0xxxcb95, _0xxxcb96, _0xxxcb97, _0xxxcb98, _0xxxcb99, _0xxxcb9a, _0xxxcb9b, _0xxxcb9c, _0xxxcb9d, _0xxxcb9e, _0xxxcb9f,
            _0xxxcba0, _0xxxcba1, _0xxxcba2, _0xxxcba3, _0xxxcba4, _0xxxcba5, _0xxxcba6, _0xxxcba7, _0xxxcba8, _0xxxcba9, _0xxxcbaa, _0xxxcbab, _0xxxcbac, _0xxxcbad, _0xxxcbae, _0xxxcbaf,
            _0xxxcbb0, _0xxxcbb1, _0xxxcbb2, _0xxxcbb3, _0xxxcbb4, _0xxxcbb5, _0xxxcbb6, _0xxxcbb7, _0xxxcbb8, _0xxxcbb9, _0xxxcbba, _0xxxcbbb, _0xxxcbbc, _0xxxcbbd, _0xxxcbbe, _0xxxcbbf,
            _0xxxcbc0, _0xxxcbc1, _0xxxcbc2, _0xxxcbc3, _0xxxcbc4, _0xxxcbc5, _0xxxcbc6, _0xxxcbc7, _0xxxcbc8, _0xxxcbc9, _0xxxcbca, _0xxxcbcb, _0xxxcbcc, _0xxxcbcd, _0xxxcbce, _0xxxcbcf,
            _0xxxcbd0, _0xxxcbd1, _0xxxcbd2, _0xxxcbd3, _0xxxcbd4, _0xxxcbd5, _0xxxcbd6, _0xxxcbd7, _0xxxcbd8, _0xxxcbd9, _0xxxcbda, _0xxxcbdb, _0xxxcbdc, _0xxxcbdd, _0xxxcbde, _0xxxcbdf,
            _0xxxcbe0, _0xxxcbe1, _0xxxcbe2, _0xxxcbe3, _0xxxcbe4, _0xxxcbe5, _0xxxcbe6, _0xxxcbe7, _0xxxcbe8, _0xxxcbe9, _0xxxcbea, _0xxxcbeb, _0xxxcbec, _0xxxcbed, _0xxxcbee, _0xxxcbef,
            _0xxxcbf0, _0xxxcbf1, _0xxxcbf2, _0xxxcbf3, _0xxxcbf4, _0xxxcbf5, _0xxxcbf6, _0xxxcbf7, _0xxxcbf8, _0xxxcbf9, _0xxxcbfa, _0xxxcbfb, _0xxxcbfc, _0xxxcbfd, _0xxxcbfe, _0xxxcbff
       };

    }

    Z80Registers registers;

    void reset()
    {
        if (sz53 == nullptr)
        {
            sz53 = new uint8_t[0x100]{};
            sz53p = new uint8_t[0x100]{};
            parity = new uint8_t[0x100]{};

            for (int i = 0; i < 0x100; ++i)
            {
                sz53[i] = i & (FLAG_3 | FLAG_5 | FLAG_S);

                int j = i;
                int p = 0;
                for (int k = 0; k < 8; k++)
                {
                    p ^= (j & 1);
                    j >>= 1;
                }

                parity[i] = (p > 0 ? 0 : FLAG_P);

                sz53p[i] = sz53[i] | parity[i];
            }

            sz53[0] |= FLAG_Z;
            sz53p[0] |= FLAG_Z;
        }

        AF = AF_= 0xffff;
        I = R = R7 = 0;
        PC = 0;
        SP = 0xffff;
        IFF1 = IFF2 = IM = 0;
        halted = false;
        iff2Read = false;
        interruptJustEnabled = false;
        Q = 0;
        BC = DE = HL = 0;
        BC_= DE_= HL_= 0;
        IX = IY = 0;
        MP = 0;

        win_app::info("Z80 -> reset()");
    }

    void cleanUp()
    {
        delete[] sz53;
        delete[] sz53p;
        delete[] parity;
    }

    void executeInstruction()
    {
        opCode = memoryRead(PC);
        CONTEND(0, 1);

        iff2Read = false;
        interruptJustEnabled = false;

        do
        {
            retryOpCode = false;

            ++R;
            ++PC;
            lastQ = Q;
            Q = 0;

            opCodes[opCode]();
        }
        while (retryOpCode);
    }

    bool requestInterrupt()
    {
        if (!IFF1 || interruptJustEnabled)
        {
            return false;
        }

        if ( iff2Read && !IS_CMOS)
        {
            F &= ~FLAG_P;
        }

        if (halted)
        {
            ++PC;
            halted = false;
        }

        IFF1 = IFF2 = false;

        ++R;

        ula::contendedTacts(0, 7);

        memoryWrite(--SP, PCH);
        memoryWrite(--SP, PCL);

        switch(IM)
        {
        case 0:
            PC = 0x0038;
            break;
        case 1:
            PC = 0x0038;
            break;
        case 2:
        {
            uint16_t vec = (0x100 * I) + 0xff;
            PCL = memoryRead(vec++);
            PCH = memoryRead(vec);
            break;
        }
        }

        MP = PC;
        Q = 0;

        return true;
    }

    void xorByte(uint8_t byte)
    {
        XOR(byte);
    }
}