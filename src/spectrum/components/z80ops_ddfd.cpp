case 0x09: /* ADD REG,BC        */ REP_7(CONTEND_READ_NO_MREQ(IR, 1)); ADDW(REG, BC); break;
case 0x19: /* ADD REG,DE        */ REP_7(CONTEND_READ_NO_MREQ(IR, 1)); ADDW(REG, DE); break;
case 0x21: /* LD REG,nnnn       */ REGL = peekByte(PC++); REGH = peekByte(PC++); break;
case 0x22: /* LD (nnnn),REG     */ LDW_NNRR(REGL, REGH); break;
case 0x23: /* INC REG           */ REP_2(CONTEND_READ_NO_MREQ(IR, 1)); ++REG; break;
case 0x24: /* INC REGH          */ INC(REGH); break;
case 0x25: /* DEC REGH          */ DEC(REGH); break;
case 0x26: /* LD REGH,nn        */ REGH = peekByte(PC++); break;
case 0x29: /* ADD REG,REG       */ REP_7(CONTEND_READ_NO_MREQ(IR, 1)); ADDW(REG, REG); break;
case 0x2a: /* LD REG,(nnnn)     */ LDW_RRNN(REGL, REGH); break;
case 0x2b: /* DEC REG           */ REP_2(CONTEND_READ_NO_MREQ(IR, 1)); --REG;  break;
case 0x2c: /* INC REGL          */ INC(REGL); break;
case 0x2d: /* DEC REGL          */ DEC(REGL); break;
case 0x2e: /* LD REGL,nn        */ REGL = peekByte(PC++); break;
case 0x34: /* INC (REG+dd)      */ { uint8_t o = peekByte(PC), t; REP_5(CONTEND_READ_NO_MREQ(PC, 1)); ++PC; MP = REG + (int8_t) o; t = peekByte(MP); CONTEND_READ_NO_MREQ(MP, 1); INC(t); pokeByte(MP, t); } break;
case 0x35: /* DEC (REG+dd)      */ { uint8_t o = peekByte(PC), t; REP_5(CONTEND_READ_NO_MREQ(PC, 1)); ++PC; MP = REG + (int8_t) o; t = peekByte(MP); CONTEND_READ_NO_MREQ(MP, 1); DEC(t); pokeByte(MP, t); } break;
case 0x36: /* LD (REG+dd),nn    */ { uint8_t o = peekByte(PC++), t = peekByte(PC); REP_2(CONTEND_READ_NO_MREQ(PC, 1)); ++PC; MP = REG + (int8_t) o; pokeByte(MP, t); } break;
case 0x39: /* ADD REG,SP        */ REP_7(CONTEND_READ_NO_MREQ(IR, 1)); ADDW(REG, SP); break;
case 0x44: /* LD B,REGH         */ B = REGH; break;
case 0x45: /* LD B,REGL         */ B = REGL; break;
case 0x46: /* LD B,(REG+dd)     */ { uint8_t o = peekByte(PC); REP_5(CONTEND_READ_NO_MREQ(PC, 1)); ++PC; MP = REG + (int8_t) o; B = peekByte(MP); } break;
case 0x4c: /* LD C,REGH         */ C = REGH; break;
case 0x4d: /* LD C,REGL         */ C = REGL; break;
case 0x4e: /* LD C,(REG+dd)     */ { uint8_t o = peekByte(PC); REP_5(CONTEND_READ_NO_MREQ(PC, 1)); ++PC; MP = REG + (int8_t) o; C = peekByte(MP); } break;
case 0x54: /* LD D,REGH         */ D = REGH; break;
case 0x55: /* LD D,REGL         */ D = REGL; break;
case 0x56: /* LD D,(REG+dd)     */ { uint8_t o = peekByte(PC); REP_5(CONTEND_READ_NO_MREQ(PC, 1)); ++PC; MP = REG + (int8_t) o; D = peekByte(MP); } break;
case 0x5c: /* LD E,REGH         */ E = REGH; break;
case 0x5d: /* LD E,REGL         */ E = REGL; break;
case 0x5e: /* LD E,(REG+dd)     */ { uint8_t o = peekByte(PC); REP_5(CONTEND_READ_NO_MREQ(PC, 1)); ++PC; MP = REG + (int8_t) o; E = peekByte(MP); } break;
case 0x60: /* LD REGH,B         */ REGH = B; break;
case 0x61: /* LD REGH,C         */ REGH = C; break;
case 0x62: /* LD REGH,D         */ REGH = D; break;
case 0x63: /* LD REGH,E         */ REGH = E; break;
case 0x64: /* LD REGH,REGH      */ break;
case 0x65: /* LD REGH,REGL      */ REGH = REGL; break;
case 0x66: /* LD H,(REG+dd)     */ { uint8_t o = peekByte(PC); REP_5(CONTEND_READ_NO_MREQ(PC, 1)); ++PC; MP = REG + (int8_t) o; H = peekByte(MP); } break;
case 0x67: /* LD REGH,A         */ REGH = A; break;
case 0x68: /* LD REGL,B         */ REGL = B; break;
case 0x69: /* LD REGL,C         */ REGL = C; break;
case 0x6a: /* LD REGL,D         */ REGL = D; break;
case 0x6b: /* LD REGL,E         */ REGL = E; break;
case 0x6c: /* LD REGL,REGH      */ REGL = REGH; break;
case 0x6d: /* LD REGL,REGL      */ break;
case 0x6e: /* LD L,(REG+dd)     */ { uint8_t o = peekByte(PC); REP_5(CONTEND_READ_NO_MREQ(PC, 1)); ++PC; MP = REG + (int8_t) o; L = peekByte(MP); } break;
case 0x6f: /* LD REGL,A         */ REGL = A; break;
case 0x70: /* LD (REG+dd),B     */ { uint8_t o = peekByte(PC); REP_5(CONTEND_READ_NO_MREQ(PC, 1)); ++PC; MP = REG + (int8_t) o; pokeByte(MP, B); } break;
case 0x71: /* LD (REG+dd),C     */ { uint8_t o = peekByte(PC); REP_5(CONTEND_READ_NO_MREQ(PC, 1)); ++PC; MP = REG + (int8_t) o; pokeByte(MP, C); } break;
case 0x72: /* LD (REG+dd),D     */ { uint8_t o = peekByte(PC); REP_5(CONTEND_READ_NO_MREQ(PC, 1)); ++PC; MP = REG + (int8_t) o; pokeByte(MP, D); } break;
case 0x73: /* LD (REG+dd),E     */ { uint8_t o = peekByte(PC); REP_5(CONTEND_READ_NO_MREQ(PC, 1)); ++PC; MP = REG + (int8_t) o; pokeByte(MP, E); } break;
case 0x74: /* LD (REG+dd),H     */ { uint8_t o = peekByte(PC); REP_5(CONTEND_READ_NO_MREQ(PC, 1)); ++PC; MP = REG + (int8_t) o; pokeByte(MP, H); } break;
case 0x75: /* LD (REG+dd),L     */ { uint8_t o = peekByte(PC); REP_5(CONTEND_READ_NO_MREQ(PC, 1)); ++PC; MP = REG + (int8_t) o; pokeByte(MP, L); } break;
case 0x77: /* LD (REG+dd),A     */ { uint8_t o = peekByte(PC); REP_5(CONTEND_READ_NO_MREQ(PC, 1)); ++PC; MP = REG + (int8_t) o; pokeByte(MP, A); } break;
case 0x7c: /* LD A,REGH         */ A = REGH; break;
case 0x7d: /* LD A,REGL         */ A = REGL; break;
case 0x7e: /* LD A,(REG+dd)     */ { uint8_t o = peekByte(PC); REP_5(CONTEND_READ_NO_MREQ(PC, 1)); ++PC; MP = REG + (int8_t) o; A = peekByte(MP); } break;
case 0x84: /* ADD A,REGH        */ ADD(REGH); break;
case 0x85: /* ADD A,REGL        */ ADD(REGL); break;
case 0x86: /* ADD A,(REG+dd)    */ { uint8_t o = peekByte(PC); REP_5(CONTEND_READ_NO_MREQ(PC, 1)); ++PC; MP = REG + (int8_t) o; uint8_t v = peekByte(MP); ADD(v); } break;
case 0x8c: /* ADC A,REGH        */ ADC(REGH); break;
case 0x8d: /* ADC A,REGL        */ ADC(REGL); break;
case 0x8e: /* ADC A,(REG+dd)    */ { uint8_t o = peekByte(PC); REP_5(CONTEND_READ_NO_MREQ(PC, 1)); ++PC; MP = REG + (int8_t) o; uint8_t v = peekByte(MP); ADC(v); } break;
case 0x94: /* SUB A,REGH        */ SUB(REGH); break;
case 0x95: /* SUB A,REGL        */ SUB(REGL); break;
case 0x96: /* SUB A,(REG+dd)    */ { uint8_t o = peekByte(PC); REP_5(CONTEND_READ_NO_MREQ(PC, 1)); ++PC; MP = REG + (int8_t) o; uint8_t v = peekByte(MP); SUB(v); } break;
case 0x9c: /* SBC A,REGH        */ SBC(REGH); break;
case 0x9d: /* SBC A,REGL        */ SBC(REGL); break;
case 0x9e: /* SBC A,(REG+dd)    */ { uint8_t o = peekByte(PC); REP_5(CONTEND_READ_NO_MREQ(PC, 1)); ++PC; MP = REG + (int8_t) o; uint8_t v = peekByte(MP); SBC(v); } break;
case 0xa4: /* AND A,REGH        */ AND(REGH); break;
case 0xa5: /* AND A,REGL        */ AND(REGL); break;
case 0xa6: /* AND A,(REG+dd)    */ { uint8_t o = peekByte(PC); REP_5(CONTEND_READ_NO_MREQ(PC, 1)); ++PC; MP = REG + (int8_t) o; uint8_t v = peekByte(MP); AND(v); } break;
case 0xac: /* XOR A,REGH        */ XOR(REGH); break;
case 0xad: /* XOR A,REGL        */ XOR(REGL); break;
case 0xae: /* XOR A,(REG+dd)    */ { uint8_t o = peekByte(PC); REP_5(CONTEND_READ_NO_MREQ(PC, 1)); ++PC; MP = REG + (int8_t) o; uint8_t v = peekByte(MP); XOR(v); } break;
case 0xb4: /* OR A,REGH         */ OR(REGH); break;
case 0xb5: /* OR A,REGL         */ OR(REGL); break;
case 0xb6: /* OR A,(REG+dd)     */ { uint8_t o = peekByte(PC); REP_5(CONTEND_READ_NO_MREQ(PC, 1)); ++PC; MP = REG + (int8_t) o; uint8_t v = peekByte(MP); OR(v); } break;
case 0xbc: /* CP A,REGH         */ CP(REGH); break;
case 0xbd: /* CP A,REGL         */ CP(REGL); break;
case 0xbe: /* CP A,(REG+dd)     */ { uint8_t o = peekByte(PC); REP_5(CONTEND_READ_NO_MREQ(PC, 1)); ++PC; MP = REG + (int8_t) o; uint8_t v = peekByte(MP); CP(v); } break;

case 0xcb: /* shift DDFDCB      */
{
    uint8_t opcode3;
    CONTEND(PC, 3);
    MP = REG + (int8_t) memory::read(PC);
    ++PC;
    CONTEND(PC, 3);
    opcode3 = memory::read(PC);
    REP_2(CONTEND_READ_NO_MREQ(PC, 1));
    ++PC;

    switch (opcode3)
    {
#include "z80ops_ddfd_cb.cpp"
    }
}
break;

case 0xe1: /* POP REG           */ POPW(REGL, REGH); break;
case 0xe3: /* EX (SP),REG       */ { uint8_t tl, th; tl = peekByte(SP); th = peekByte(SP + 1); CONTEND_READ_NO_MREQ(SP + 1, 1); pokeByte(SP + 1, REGH); pokeByte(SP, REGL); REP_2(CONTEND_WRITE_NO_MREQ(SP, 1)); REGL = MPL = tl; REGH = MPH = th; } break;
case 0xe5: /* PUSH REG          */ CONTEND_READ_NO_MREQ(IR, 1); PUSHW(REGL, REGH); break;
case 0xe9: /* JP REG            */ PC = REG; break;
case 0xf9: /* LD SP,REG         */ REP_2(CONTEND_READ_NO_MREQ(IR, 1)); SP = REG; break;