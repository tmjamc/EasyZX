case 0x40: /* IN B,(C)     */ Z80_IN(B, BC); break;
case 0x41: /* OUT (C),B    */ io::write(BC, B); MP = BC + 1; break;
case 0x42: /* SBC HL,BC    */ REP_7(CONTEND_READ_NO_MREQ(IR, 1)); SBCW(BC); break;
case 0x43: /* LD (nnnn),BC */ LDW_NNRR(C,B); break;
case 0x44:
case 0x4c:
case 0x54:
case 0x5c:
case 0x64:
case 0x6c:
case 0x74:
case 0x7c: /* NEG           */ { uint8_t v = A; A = 0; SUB(v); } break;
case 0x45:
case 0x4d:
case 0x55:
case 0x5d:
case 0x65:
case 0x6d:
case 0x75:
case 0x7d: /* RETN          */ IFF1=IFF2; RET(); break;
case 0x46:
case 0x4e:
case 0x66:
case 0x6e: /* IM 0          */ IM = 0; break;
case 0x47: /* LD I,A        */ CONTEND_READ_NO_MREQ(IR, 1); I = A; break;
case 0x48: /* IN C,(C)      */ Z80_IN(C, BC); break;
case 0x49: /* OUT (C),C     */ io::write(BC, C); MP = BC + 1; break;
case 0x4a: /* ADC HL,BC     */ REP_7(CONTEND_READ_NO_MREQ(IR, 1)); ADCW(BC); break;
case 0x4b: /* LD BC,(nnnn)  */ LDW_RRNN(C, B); break;
case 0x4f: /* LD R,A        */ CONTEND_READ_NO_MREQ(IR, 1); R = R7 = A; break;
case 0x50: /* IN D,(C)      */ Z80_IN( D, BC ); break;
case 0x51: /* OUT (C),D     */ io::write(BC, D); MP = BC + 1; break;
case 0x52: /* SBC HL,DE     */ REP_7(CONTEND_READ_NO_MREQ(IR, 1)); SBCW(DE); break;
case 0x53: /* LD (nnnn),DE  */ LDW_NNRR(E, D); break;
case 0x56:
case 0x76: /* IM 1          */ IM = 1; break;
case 0x57: /* LD A,I        */ CONTEND_READ_NO_MREQ(IR, 1); A = I; F = (F & FLAG_C) | _sz53[A] | (IFF2 ? FLAG_V : 0); Q = F; iff2Read = true; break;
case 0x58: /* IN E,(C)      */ Z80_IN(E, BC); break;
case 0x59: /* OUT (C),E     */ io::write(BC, E); MP = BC + 1; break;
case 0x5a: /* ADC HL,DE     */ REP_7(CONTEND_READ_NO_MREQ(IR, 1)); ADCW(DE); break;
case 0x5b: /* LD DE,(nnnn)  */ LDW_RRNN(E, D); break;
case 0x5e:
case 0x7e: /* IM 2          */ IM = 2; break;
case 0x5f: /* LD A,R        */ CONTEND_READ_NO_MREQ(IR, 1); A = (R & 0x7f) | (R7 & 0x80); F = (F & FLAG_C) | _sz53[A] | (IFF2 ? FLAG_V : 0); Q = F; iff2Read = true; break;
case 0x60: /* IN H,(C)      */ Z80_IN(H, BC); break;
case 0x61: /* OUT (C),H     */ io::write(BC, H); MP = BC + 1; break;
case 0x62: /* SBC HL,HL     */ REP_7(CONTEND_READ_NO_MREQ(IR, 1)); SBCW(HL); break;
case 0x63: /* LD (nnnn),HL  */ LDW_NNRR(L, H); break;
case 0x67: /* RRD           */ { uint8_t v = peekByte(HL); REP_4(CONTEND_READ_NO_MREQ(HL, 1)); pokeByte(HL, (A << 4) | (v >> 4)); A = (A & 0xf0) | (v & 0x0f); F = (F & FLAG_C) | _sz53p[A]; Q = F; MP = HL + 1; } break;
case 0x68: /* IN L,(C)      */ Z80_IN(L, BC); break;
case 0x69: /* OUT (C),L     */ io::write(BC, L); MP = BC + 1; break;
case 0x6a: /* ADC HL,HL     */ REP_7(CONTEND_READ_NO_MREQ(IR, 1)); ADCW(HL); break;
case 0x6b: /* LD HL,(nnnn)  */ LDW_RRNN(L, H); break;
case 0x6f: /* RLD           */ { uint8_t v = peekByte(HL); REP_4(CONTEND_READ_NO_MREQ(HL, 1)); pokeByte(HL, (v << 4) | (A & 0x0f)); A = (A & 0xf0) | (v >> 4 ); F = (F & FLAG_C) | _sz53p[A]; Q = F; MP = HL + 1; } break;
case 0x70: /* IN F,(C)      */ Z80_IN(F, BC); break;
case 0x71: /* OUT (C),0     */ io::write(BC, IS_CMOS ? 0xff : 0); MP = BC + 1; break;
case 0x72: /* SBC HL,SP     */ REP_7(CONTEND_READ_NO_MREQ(IR, 1)); SBCW(SP); break;
case 0x73: /* LD (nnnn),SP  */ LDW_NNRR(SPL, SPH); break;
case 0x78: /* IN A,(C)      */ Z80_IN(A, BC); break;
case 0x79: /* OUT (C),A     */ io::write(BC, A); MP = BC + 1; break;
case 0x7a: /* ADC HL,SP     */ REP_7(CONTEND_READ_NO_MREQ(IR, 1)); ADCW(SP); break;
case 0x7b: /* LD SP,(nnnn)  */ LDW_RRNN(SPL, SPH); break;
case 0xa0: /* LDI           */ { uint8_t v = peekByte(HL); BC--; pokeByte(DE, v); REP_2(CONTEND_WRITE_NO_MREQ(DE, 1)); ++DE; ++HL; v += A; F = (F & (FLAG_C | FLAG_Z | FLAG_S)) | (BC ? FLAG_V : 0) | (v & FLAG_3) | ((v & 0x02) ? FLAG_5 : 0); Q = F; }; break;
case 0xa1: /* CPI           */ { uint8_t v = peekByte(HL), t = A - v, l = ((A & 0x08) >> 3) | ((v & 0x08) >> 2) | ((t & 0x08) >> 1); REP_5(CONTEND_READ_NO_MREQ(HL, 1)); ++HL; --BC; F = (F & FLAG_C) | (BC ? (FLAG_V | FLAG_N) : FLAG_N) | _halfcarrySub[l] | (t ? 0 : FLAG_Z) | (t & FLAG_S); if (F & FLAG_H) t--; F |= (t & FLAG_3) | ((t & 0x02) ? FLAG_5 : 0); Q = F; ++MP; } break;
case 0xa2: /* INI           */ { uint8_t t1, t2; CONTEND_READ_NO_MREQ(IR, 1); t1 = io::read(BC); pokeByte(HL, t1); MP = BC + 1; --B; ++HL; t2 = t1 + C + 1; F = ((t1 & 0x80) ? FLAG_N : 0) | ((t2 < t1) ? FLAG_H | FLAG_C : 0) | ( _parity[(t2 & 0x07) ^ B] ? FLAG_P : 0) | _sz53[B]; Q = F; } break;
case 0xa3: /* OUTI          */ { uint8_t t1, t2; CONTEND_READ_NO_MREQ(IR, 1); t1 = peekByte(HL); --B; MP = BC + 1; io::write(BC, t1); ++HL; t2 = t1 + L; F = ((t1 & 0x80) ? FLAG_N : 0) |((t2 < t1) ? FLAG_H | FLAG_C : 0) | (_parity[(t2 & 0x07) ^ B] ? FLAG_P : 0) | _sz53[B]; Q = F; } break;
case 0xa8: /* LDD           */ { uint8_t t = peekByte(HL); --BC; pokeByte(DE, t); REP_2(CONTEND_WRITE_NO_MREQ(DE, 1)); --DE; --HL; t += A; F = (F & (FLAG_C | FLAG_Z | FLAG_S)) | (BC ? FLAG_V : 0) | (t & FLAG_3) | ((t & 0x02) ? FLAG_5 : 0); Q = F; } break;
case 0xa9: /* CPD           */ { uint8_t v = peekByte(HL), t = A - v, l = ((A & 0x08) >> 3) | ((v & 0x08) >> 2 ) | ((t & 0x08) >> 1); REP_5(CONTEND_READ_NO_MREQ(HL, 1)); --HL; --BC; F = (F & FLAG_C) | (BC ? (FLAG_V | FLAG_N) : FLAG_N) | _halfcarrySub[l] | (t ? 0 : FLAG_Z) | (t & FLAG_S); if (F & FLAG_H) --t; F |= (t & FLAG_3) | ((t & 0x02) ? FLAG_5 : 0); Q = F; --MP; } break;
case 0xaa: /* IND           */ { uint8_t t1, t2; CONTEND_READ_NO_MREQ(IR, 1); t1 = io::read(BC); pokeByte(HL, t1); MP = BC - 1; --B; --HL; t2 = t1 + C - 1; F = ((t1 & 0x80) ? FLAG_N : 0) | ((t2 < t1) ? FLAG_H | FLAG_C : 0) | (_parity[(t2 & 0x07) ^ B] ? FLAG_P : 0) | _sz53[B]; Q = F; } break;
case 0xab: /* OUTD          */ { uint8_t t1, t2; CONTEND_READ_NO_MREQ(IR, 1); t1 = peekByte(HL); --B; MP = BC - 1; io::write(BC, t1); --HL; t2 = t1 + L; F = ((t1 & 0x80) ? FLAG_N : 0) | ((t2 < t1) ? FLAG_H | FLAG_C : 0) | (_parity[(t2 & 0x07) ^ B] ? FLAG_P : 0) | _sz53[B]; Q = F; } break;
case 0xb0: /* LDIR          */ { uint8_t t = peekByte(HL); pokeByte(DE, t); REP_2(CONTEND_WRITE_NO_MREQ(DE, 1)); --BC; t += A; F = (F & (FLAG_C | FLAG_Z | FLAG_S)) | (BC ? FLAG_V : 0) | (t & FLAG_3) | ((t & 0x02) ? FLAG_5 : 0); Q = F; if (BC) { REP_5(CONTEND_WRITE_NO_MREQ(DE, 1)); PC -= 2; MP = PC + 1; } ++HL; ++DE; } break;
case 0xb1: /* CPIR          */ { uint8_t v = peekByte(HL), t = A - v, l = ((A & 0x08) >> 3) | ((v & 0x08) >> 2) | ((t & 0x08) >> 1); REP_5(CONTEND_READ_NO_MREQ(HL, 1)); --BC; F = (F & FLAG_C) | (BC ? (FLAG_V | FLAG_N) : FLAG_N) | _halfcarrySub[l] | (t ? 0 : FLAG_Z) | (t & FLAG_S); if (F & FLAG_H) --t; F |= (t & FLAG_3) | ((t & 0x02) ? FLAG_5 : 0); Q = F; if ((F & (FLAG_V | FLAG_Z)) == FLAG_V) { REP_5(CONTEND_READ_NO_MREQ(HL, 1)); PC -= 2; MP = PC + 1; } else { ++MP; } ++HL; } break;
case 0xb2: /* INIR          */ { uint8_t t1, t2; CONTEND_READ_NO_MREQ(IR, 1); t1 =io::read(BC); pokeByte(HL, t1); MP = BC + 1; --B; t2 = t1 + C + 1; F = ((t1 & 0x80) ? FLAG_N : 0) | ((t2 < t1) ? FLAG_H | FLAG_C : 0) | (_parity[(t2 & 0x07) ^ B] ? FLAG_P : 0) | _sz53[B]; Q = F; if (B) { REP_5(CONTEND_WRITE_NO_MREQ(HL, 1)); PC -= 2; } ++HL; } break;
case 0xb3: /* OTIR          */ { uint8_t t1, t2; CONTEND_READ_NO_MREQ(IR, 1); t1 = peekByte(HL); --B; MP = BC + 1; io::write(BC, t1); ++HL; t2 = t1 + L; F = ((t1 & 0x80) ? FLAG_N : 0) | ((t2 < t1) ? FLAG_H | FLAG_C : 0) | (_parity[(t2 & 0x07) ^ B] ? FLAG_P : 0) | _sz53[B]; Q = F; if(B) { REP_5(CONTEND_READ_NO_MREQ(BC, 1)); PC -= 2; } } break;
case 0xb8: /* LDDR          */ { uint8_t b = peekByte(HL); pokeByte(DE, b); REP_2(CONTEND_WRITE_NO_MREQ(DE, 1)); --BC; b += A; F = (F & (FLAG_C | FLAG_Z | FLAG_S)) | (BC ? FLAG_V : 0) | (b & FLAG_3) | ((b & 0x02) ? FLAG_5 : 0); Q = F; if (BC) { REP_5(CONTEND_WRITE_NO_MREQ(DE, 1)); PC -= 2; MP = PC + 1; } --HL; --DE; } break;
case 0xb9: /* CPDR          */ { uint8_t v = peekByte(HL), b = A - v, l = ((A & 0x08) >> 3) | ((v & 0x08) >> 2) | ((b & 0x08) >> 1); REP_5(CONTEND_READ_NO_MREQ(HL, 1)); --BC; F = (F & FLAG_C) | (BC ? (FLAG_V | FLAG_N) : FLAG_N) | _halfcarrySub[l] | (b ? 0 : FLAG_Z) | (b & FLAG_S); if (F & FLAG_H) --b; F |= (b & FLAG_3) | ((b & 0x02) ? FLAG_5 : 0); Q = F; if ((F & (FLAG_V | FLAG_Z)) == FLAG_V) { REP_5(CONTEND_READ_NO_MREQ(HL, 1)); PC -= 2; MP = PC + 1; } else { --MP; } --HL; } break;
case 0xba: /* INDR          */ { uint8_t t1, t2; CONTEND_READ_NO_MREQ(IR, 1); t1 = io::read(BC); pokeByte(HL, t1); MP = BC - 1; --B; t2 = t1 + C - 1; F = ((t1 & 0x80) ? FLAG_N : 0) | ((t2 < t1) ? FLAG_H | FLAG_C : 0) | (_parity[(t2 & 0x07) ^ B] ? FLAG_P : 0) | _sz53[B]; Q = F; if (B) { REP_5(CONTEND_WRITE_NO_MREQ(HL, 1)); PC -= 2; } --HL; } break;
case 0xbb: /* OTDR          */ { uint8_t t1, t2; CONTEND_READ_NO_MREQ(IR, 1); t1 = peekByte(HL); --B; MP = BC - 1; io::write(BC, t1); --HL; t2 = t1 + L; F = ((t1 & 0x80) ? FLAG_N : 0) | ((t2 < t1) ? FLAG_H | FLAG_C : 0) | (_parity[(t2 & 0x07) ^ B] ? FLAG_P : 0) | _sz53[B]; Q = F; if (B) { REP_5(CONTEND_READ_NO_MREQ(BC, 1)); PC -= 2; } } break;
case 0xfb: /* slttrap       */ break;
default:   /* NOPD          */ break;