case 0x00: /* LD B,RLC (REG+dd)   */ B = contendRead(MP); CONTEND_READ_NO_MREQ(MP, 1); RLC(B); contendWrite(MP, B); break;
case 0x01: /* LD C,RLC (REG+dd)   */ C = contendRead(MP); CONTEND_READ_NO_MREQ(MP, 1); RLC(C); contendWrite(MP, C); break;
case 0x02: /* LD D,RLC (REG+dd)   */ D = contendRead(MP); CONTEND_READ_NO_MREQ(MP, 1); RLC(D); contendWrite(MP, D); break;
case 0x03: /* LD E,RLC (REG+dd)   */ E = contendRead(MP); CONTEND_READ_NO_MREQ(MP, 1); RLC(E); contendWrite(MP, E); break;
case 0x04: /* LD H,RLC (REG+dd)   */ H = contendRead(MP); CONTEND_READ_NO_MREQ(MP, 1); RLC(H); contendWrite(MP, H); break;
case 0x05: /* LD L,RLC (REG+dd)   */ L = contendRead(MP); CONTEND_READ_NO_MREQ(MP, 1); RLC(L); contendWrite(MP, L); break;
case 0x06: /* RLC (REG+dd)        */ { uint8_t t = contendRead(MP); CONTEND_READ_NO_MREQ(MP, 1); RLC(t); contendWrite(MP, t); } break;
case 0x07: /* LD A,RLC (REG+dd)   */ A = contendRead(MP); CONTEND_READ_NO_MREQ(MP, 1); RLC(A); contendWrite(MP, A); break;
case 0x08: /* LD B,RRC (REG+dd)   */ B = contendRead(MP); CONTEND_READ_NO_MREQ(MP, 1); RRC(B); contendWrite(MP, B); break;
case 0x09: /* LD C,RRC (REG+dd)   */ C = contendRead(MP); CONTEND_READ_NO_MREQ(MP, 1); RRC(C); contendWrite(MP, C); break;
case 0x0a: /* LD D,RRC (REG+dd)   */ D = contendRead(MP); CONTEND_READ_NO_MREQ(MP, 1); RRC(D); contendWrite(MP, D); break;
case 0x0b: /* LD E,RRC (REG+dd)   */ E = contendRead(MP); CONTEND_READ_NO_MREQ(MP, 1); RRC(E); contendWrite(MP, E); break;
case 0x0c: /* LD H,RRC (REG+dd)   */ H = contendRead(MP); CONTEND_READ_NO_MREQ(MP, 1); RRC(H); contendWrite(MP, H); break;
case 0x0d: /* LD L,RRC (REG+dd)   */ L = contendRead(MP); CONTEND_READ_NO_MREQ(MP, 1); RRC(L); contendWrite(MP, L); break;
case 0x0e: /* RRC (REG+dd)        */ { uint8_t t = contendRead(MP); CONTEND_READ_NO_MREQ(MP, 1); RRC(t); contendWrite(MP, t); } break;
case 0x0f: /* LD A,RRC (REG+dd)   */ A = contendRead(MP); CONTEND_READ_NO_MREQ(MP, 1); RRC(A); contendWrite(MP, A); break;
case 0x10: /* LD B,RL (REG+dd)    */ B = contendRead(MP); CONTEND_READ_NO_MREQ(MP, 1); RL(B); contendWrite(MP, B); break;
case 0x11: /* LD C,RL (REG+dd)    */ C = contendRead(MP); CONTEND_READ_NO_MREQ(MP, 1); RL(C); contendWrite(MP, C); break;
case 0x12: /* LD D,RL (REG+dd)    */ D = contendRead(MP); CONTEND_READ_NO_MREQ(MP, 1); RL(D); contendWrite(MP, D); break;
case 0x13: /* LD E,RL (REG+dd)    */ E = contendRead(MP); CONTEND_READ_NO_MREQ(MP, 1); RL(E); contendWrite(MP, E); break;
case 0x14: /* LD H,RL (REG+dd)    */ H = contendRead(MP); CONTEND_READ_NO_MREQ(MP, 1); RL(H); contendWrite(MP, H); break;
case 0x15: /* LD L,RL (REG+dd)    */ L = contendRead(MP); CONTEND_READ_NO_MREQ(MP, 1); RL(L); contendWrite(MP, L); break;
case 0x16: /* RL (REG+dd)         */ { uint8_t t = contendRead(MP); CONTEND_READ_NO_MREQ(MP, 1); RL(t); contendWrite(MP, t); } break;
case 0x17: /* LD A,RL (REG+dd)    */ A = contendRead(MP); CONTEND_READ_NO_MREQ(MP, 1); RL(A); contendWrite(MP, A); break;
case 0x18: /* LD B,RR (REG+dd)    */ B = contendRead(MP); CONTEND_READ_NO_MREQ(MP, 1); RR(B); contendWrite(MP, B); break;
case 0x19: /* LD C,RR (REG+dd)    */ C = contendRead(MP); CONTEND_READ_NO_MREQ(MP, 1); RR(C); contendWrite(MP, C); break;
case 0x1a: /* LD D,RR (REG+dd)    */ D = contendRead(MP); CONTEND_READ_NO_MREQ(MP, 1); RR(D); contendWrite(MP, D); break;
case 0x1b: /* LD E,RR (REG+dd)    */ E = contendRead(MP); CONTEND_READ_NO_MREQ(MP, 1); RR(E); contendWrite(MP, E); break;
case 0x1c: /* LD H,RR (REG+dd)    */ H = contendRead(MP); CONTEND_READ_NO_MREQ(MP, 1); RR(H); contendWrite(MP, H); break;
case 0x1d: /* LD L,RR (REG+dd)    */ L = contendRead(MP); CONTEND_READ_NO_MREQ(MP, 1); RR(L); contendWrite(MP, L); break;
case 0x1e: /* RR (REG+dd)         */ { uint8_t t = contendRead(MP); CONTEND_READ_NO_MREQ(MP, 1); RR(t); contendWrite(MP, t); } break;
case 0x1f: /* LD A,RR (REG+dd)    */ A = contendRead(MP); CONTEND_READ_NO_MREQ(MP, 1); RR(A); contendWrite(MP, A); break;
case 0x20: /* LD B,SLA (REG+dd)   */ B = contendRead(MP); CONTEND_READ_NO_MREQ(MP, 1); SLA(B); contendWrite(MP, B); break;
case 0x21: /* LD C,SLA (REG+dd)   */ C = contendRead(MP); CONTEND_READ_NO_MREQ(MP, 1); SLA(C); contendWrite(MP, C); break;
case 0x22: /* LD D,SLA (REG+dd)   */ D = contendRead(MP); CONTEND_READ_NO_MREQ(MP, 1); SLA(D); contendWrite(MP, D); break;
case 0x23: /* LD E,SLA (REG+dd)   */ E = contendRead(MP); CONTEND_READ_NO_MREQ(MP, 1); SLA(E); contendWrite(MP, E); break;
case 0x24: /* LD H,SLA (REG+dd)   */ H = contendRead(MP); CONTEND_READ_NO_MREQ(MP, 1); SLA(H); contendWrite(MP, H); break;
case 0x25: /* LD L,SLA (REG+dd)   */ L = contendRead(MP); CONTEND_READ_NO_MREQ(MP, 1); SLA(L); contendWrite(MP, L); break;
case 0x26: /* SLA (REG+dd)        */ { uint8_t t = contendRead(MP); CONTEND_READ_NO_MREQ(MP, 1); SLA(t); contendWrite(MP, t); } break;
case 0x27: /* LD A,SLA (REG+dd)   */ A = contendRead(MP); CONTEND_READ_NO_MREQ(MP, 1); SLA(A); contendWrite(MP, A); break;
case 0x28: /* LD B,SRA (REG+dd)   */ B = contendRead(MP); CONTEND_READ_NO_MREQ(MP, 1); SRA(B); contendWrite(MP, B); break;
case 0x29: /* LD C,SRA (REG+dd)   */ C = contendRead(MP); CONTEND_READ_NO_MREQ(MP, 1); SRA(C); contendWrite(MP, C); break;
case 0x2a: /* LD D,SRA (REG+dd)   */ D = contendRead(MP); CONTEND_READ_NO_MREQ(MP, 1); SRA(D); contendWrite(MP, D); break;
case 0x2b: /* LD E,SRA (REG+dd)   */ E = contendRead(MP); CONTEND_READ_NO_MREQ(MP, 1); SRA(E); contendWrite(MP, E); break;
case 0x2c: /* LD H,SRA (REG+dd)   */ H = contendRead(MP); CONTEND_READ_NO_MREQ(MP, 1); SRA(H); contendWrite(MP, H); break;
case 0x2d: /* LD L,SRA (REG+dd)   */ L = contendRead(MP); CONTEND_READ_NO_MREQ(MP, 1); SRA(L); contendWrite(MP, L); break;
case 0x2e: /* SRA (REG+dd)        */ { uint8_t t = contendRead(MP); CONTEND_READ_NO_MREQ(MP, 1); SRA(t); contendWrite(MP, t); } break;
case 0x2f: /* LD A,SRA (REG+dd)   */ A = contendRead(MP); CONTEND_READ_NO_MREQ(MP, 1); SRA(A); contendWrite(MP, A); break;
case 0x30: /* LD B,SLL (REG+dd)   */ B = contendRead(MP); CONTEND_READ_NO_MREQ(MP, 1); SLL(B); contendWrite(MP, B); break;
case 0x31: /* LD C,SLL (REG+dd)   */ C = contendRead(MP); CONTEND_READ_NO_MREQ(MP, 1); SLL(C); contendWrite(MP, C); break;
case 0x32: /* LD D,SLL (REG+dd)   */ D = contendRead(MP); CONTEND_READ_NO_MREQ(MP, 1); SLL(D); contendWrite(MP, D); break;
case 0x33: /* LD E,SLL (REG+dd)   */ E = contendRead(MP); CONTEND_READ_NO_MREQ(MP, 1); SLL(E); contendWrite(MP, E); break;
case 0x34: /* LD H,SLL (REG+dd)   */ H = contendRead(MP); CONTEND_READ_NO_MREQ(MP, 1); SLL(H); contendWrite(MP, H); break;
case 0x35: /* LD L,SLL (REG+dd)   */ L = contendRead(MP); CONTEND_READ_NO_MREQ(MP, 1); SLL(L); contendWrite(MP, L); break;
case 0x36: /* SLL (REG+dd)        */ { uint8_t t = contendRead(MP); CONTEND_READ_NO_MREQ(MP, 1); SLL(t); contendWrite(MP, t); } break;
case 0x37: /* LD A,SLL (REG+dd)   */ A = contendRead(MP); CONTEND_READ_NO_MREQ(MP, 1); SLL(A); contendWrite(MP, A); break;
case 0x38: /* LD B,SRL (REG+dd)   */ B = contendRead(MP); CONTEND_READ_NO_MREQ(MP, 1); SRL(B); contendWrite(MP, B); break;
case 0x39: /* LD C,SRL (REG+dd)   */ C = contendRead(MP); CONTEND_READ_NO_MREQ(MP, 1); SRL(C); contendWrite(MP, C); break;
case 0x3a: /* LD D,SRL (REG+dd)   */ D = contendRead(MP); CONTEND_READ_NO_MREQ(MP, 1); SRL(D); contendWrite(MP, D); break;
case 0x3b: /* LD E,SRL (REG+dd)   */ E = contendRead(MP); CONTEND_READ_NO_MREQ(MP, 1); SRL(E); contendWrite(MP, E); break;
case 0x3c: /* LD H,SRL (REG+dd)   */ H = contendRead(MP); CONTEND_READ_NO_MREQ(MP, 1); SRL(H); contendWrite(MP, H); break;
case 0x3d: /* LD L,SRL (REG+dd)   */ L = contendRead(MP); CONTEND_READ_NO_MREQ(MP, 1); SRL(L); contendWrite(MP, L); break;
case 0x3e: /* SRL (REG+dd)        */ { uint8_t t = contendRead(MP); CONTEND_READ_NO_MREQ(MP, 1); SRL(t); contendWrite(MP, t); } break;
case 0x3f: /* LD A,SRL (REG+dd)   */ A = contendRead(MP); CONTEND_READ_NO_MREQ(MP, 1); SRL(A); contendWrite(MP, A); break;
case 0x40:
case 0x41:
case 0x42:
case 0x43:
case 0x44:
case 0x45:
case 0x46:
case 0x47: /* BIT 0,(REG+dd)      */ { uint8_t t = contendRead(MP); CONTEND_READ_NO_MREQ(MP, 1); BIT_MP(0, t); } break;
case 0x48:
case 0x49:
case 0x4a:
case 0x4b:
case 0x4c:
case 0x4d:
case 0x4e:
case 0x4f: /* BIT 1,(REG+dd)      */ { uint8_t t = contendRead(MP); CONTEND_READ_NO_MREQ(MP, 1); BIT_MP(1, t); } break;
case 0x50:
case 0x51:
case 0x52:
case 0x53:
case 0x54:
case 0x55:
case 0x56:
case 0x57: /* BIT 2,(REG+dd)      */ { uint8_t t = contendRead(MP); CONTEND_READ_NO_MREQ(MP, 1); BIT_MP(2, t); } break;
case 0x58:
case 0x59:
case 0x5a:
case 0x5b:
case 0x5c:
case 0x5d:
case 0x5e:
case 0x5f: /* BIT 3,(REG+dd)      */ { uint8_t t = contendRead(MP); CONTEND_READ_NO_MREQ(MP, 1); BIT_MP(3, t); } break;
case 0x60:
case 0x61:
case 0x62:
case 0x63:
case 0x64:
case 0x65:
case 0x66:
case 0x67: /* BIT 4,(REG+dd)      */ { uint8_t t = contendRead(MP); CONTEND_READ_NO_MREQ(MP, 1); BIT_MP(4, t); } break;
case 0x68:
case 0x69:
case 0x6a:
case 0x6b:
case 0x6c:
case 0x6d:
case 0x6e:
case 0x6f: /* BIT 5,(REG+dd)      */ { uint8_t t = contendRead(MP); CONTEND_READ_NO_MREQ(MP, 1); BIT_MP(5, t); } break;
case 0x70:
case 0x71:
case 0x72:
case 0x73:
case 0x74:
case 0x75:
case 0x76:
case 0x77: /* BIT 6,(REG+dd)      */ { uint8_t t = contendRead(MP); CONTEND_READ_NO_MREQ(MP, 1); BIT_MP(6, t); } break;
case 0x78:
case 0x79:
case 0x7a:
case 0x7b:
case 0x7c:
case 0x7d:
case 0x7e:
case 0x7f: /* BIT 7,(REG+dd)      */ { uint8_t t = contendRead(MP); CONTEND_READ_NO_MREQ(MP, 1); BIT_MP(7, t); } break;
case 0x80: /* LD B,RES 0,(REG+dd) */ B = contendRead(MP) & 0xfe; CONTEND_READ_NO_MREQ(MP, 1); contendWrite(MP, B); break;
case 0x81: /* LD C,RES 0,(REG+dd) */ C = contendRead(MP) & 0xfe; CONTEND_READ_NO_MREQ(MP, 1); contendWrite(MP, C); break;
case 0x82: /* LD D,RES 0,(REG+dd) */ D = contendRead(MP) & 0xfe; CONTEND_READ_NO_MREQ(MP, 1); contendWrite(MP, D); break;
case 0x83: /* LD E,RES 0,(REG+dd) */ E = contendRead(MP) & 0xfe; CONTEND_READ_NO_MREQ(MP, 1); contendWrite(MP, E); break;
case 0x84: /* LD H,RES 0,(REG+dd) */ H = contendRead(MP) & 0xfe; CONTEND_READ_NO_MREQ(MP, 1); contendWrite(MP, H); break;
case 0x85: /* LD L,RES 0,(REG+dd) */ L = contendRead(MP) & 0xfe; CONTEND_READ_NO_MREQ(MP, 1); contendWrite(MP, L); break;
case 0x86: /* RES 0,(REG+dd)      */ { uint8_t t = contendRead(MP); CONTEND_READ_NO_MREQ(MP, 1); contendWrite(MP, t & 0xfe); } break;
case 0x87: /* LD A,RES 0,(REG+dd) */ A = contendRead(MP) & 0xfe; CONTEND_READ_NO_MREQ(MP, 1); contendWrite(MP, A); break;
case 0x88: /* LD B,RES 1,(REG+dd) */ B = contendRead(MP) & 0xfd; CONTEND_READ_NO_MREQ(MP, 1); contendWrite(MP, B); break;
case 0x89: /* LD C,RES 1,(REG+dd) */ C = contendRead(MP) & 0xfd; CONTEND_READ_NO_MREQ(MP, 1); contendWrite(MP, C); break;
case 0x8a: /* LD D,RES 1,(REG+dd) */ D = contendRead(MP) & 0xfd; CONTEND_READ_NO_MREQ(MP, 1); contendWrite(MP, D); break;
case 0x8b: /* LD E,RES 1,(REG+dd) */ E = contendRead(MP) & 0xfd; CONTEND_READ_NO_MREQ(MP, 1); contendWrite(MP, E); break;
case 0x8c: /* LD H,RES 1,(REG+dd) */ H = contendRead(MP) & 0xfd; CONTEND_READ_NO_MREQ(MP, 1); contendWrite(MP, H); break;
case 0x8d: /* LD L,RES 1,(REG+dd) */ L = contendRead(MP) & 0xfd; CONTEND_READ_NO_MREQ(MP, 1); contendWrite(MP, L); break;
case 0x8e: /* RES 1,(REG+dd)      */ { uint8_t t = contendRead(MP); CONTEND_READ_NO_MREQ(MP, 1); contendWrite(MP, t & 0xfd); } break;
case 0x8f: /* LD A,RES 1,(REG+dd) */ A = contendRead(MP) & 0xfd; CONTEND_READ_NO_MREQ(MP, 1); contendWrite(MP, A); break;
case 0x90: /* LD B,RES 2,(REG+dd) */ B = contendRead(MP) & 0xfb; CONTEND_READ_NO_MREQ(MP, 1); contendWrite(MP, B); break;
case 0x91: /* LD C,RES 2,(REG+dd) */ C = contendRead(MP) & 0xfb; CONTEND_READ_NO_MREQ(MP, 1); contendWrite(MP, C); break;
case 0x92: /* LD D,RES 2,(REG+dd) */ D = contendRead(MP) & 0xfb; CONTEND_READ_NO_MREQ(MP, 1); contendWrite(MP, D); break;
case 0x93: /* LD E,RES 2,(REG+dd) */ E = contendRead(MP) & 0xfb; CONTEND_READ_NO_MREQ(MP, 1); contendWrite(MP, E); break;
case 0x94: /* LD H,RES 2,(REG+dd) */ H = contendRead(MP) & 0xfb; CONTEND_READ_NO_MREQ(MP, 1); contendWrite(MP, H); break;
case 0x95: /* LD L,RES 2,(REG+dd) */ L = contendRead(MP) & 0xfb; CONTEND_READ_NO_MREQ(MP, 1); contendWrite(MP, L); break;
case 0x96: /* RES 2,(REG+dd)      */ { uint8_t t = contendRead(MP); CONTEND_READ_NO_MREQ(MP, 1); contendWrite(MP, t & 0xfb); } break;
case 0x97: /* LD A,RES 2,(REG+dd) */ A = contendRead(MP) & 0xfb; CONTEND_READ_NO_MREQ(MP, 1); contendWrite(MP, A); break;
case 0x98: /* LD B,RES 3,(REG+dd) */ B = contendRead(MP) & 0xf7; CONTEND_READ_NO_MREQ(MP, 1); contendWrite(MP, B); break;
case 0x99: /* LD C,RES 3,(REG+dd) */ C = contendRead(MP) & 0xf7; CONTEND_READ_NO_MREQ(MP, 1); contendWrite(MP, C); break;
case 0x9a: /* LD D,RES 3,(REG+dd) */ D = contendRead(MP) & 0xf7; CONTEND_READ_NO_MREQ(MP, 1); contendWrite(MP, D); break;
case 0x9b: /* LD E,RES 3,(REG+dd) */ E = contendRead(MP) & 0xf7; CONTEND_READ_NO_MREQ(MP, 1); contendWrite(MP, E); break;
case 0x9c: /* LD H,RES 3,(REG+dd) */ H = contendRead(MP) & 0xf7; CONTEND_READ_NO_MREQ(MP, 1); contendWrite(MP, H); break;
case 0x9d: /* LD L,RES 3,(REG+dd) */ L = contendRead(MP) & 0xf7; CONTEND_READ_NO_MREQ(MP, 1); contendWrite(MP, L); break;
case 0x9e: /* RES 3,(REG+dd)      */ { uint8_t t = contendRead(MP); CONTEND_READ_NO_MREQ(MP, 1); contendWrite(MP, t & 0xf7); } break;
case 0x9f: /* LD A,RES 3,(REG+dd) */ A = contendRead(MP) & 0xf7; CONTEND_READ_NO_MREQ(MP, 1); contendWrite(MP, A); break;
case 0xa0: /* LD B,RES 4,(REG+dd) */ B = contendRead(MP) & 0xef; CONTEND_READ_NO_MREQ(MP, 1); contendWrite(MP, B); break;
case 0xa1: /* LD C,RES 4,(REG+dd) */ C = contendRead(MP) & 0xef; CONTEND_READ_NO_MREQ(MP, 1); contendWrite(MP, C); break;
case 0xa2: /* LD D,RES 4,(REG+dd) */ D = contendRead(MP) & 0xef; CONTEND_READ_NO_MREQ(MP, 1); contendWrite(MP, D); break;
case 0xa3: /* LD E,RES 4,(REG+dd) */ E = contendRead(MP) & 0xef; CONTEND_READ_NO_MREQ(MP, 1); contendWrite(MP, E); break;
case 0xa4: /* LD H,RES 4,(REG+dd) */ H = contendRead(MP) & 0xef; CONTEND_READ_NO_MREQ(MP, 1); contendWrite(MP, H); break;
case 0xa5: /* LD L,RES 4,(REG+dd) */ L = contendRead(MP) & 0xef; CONTEND_READ_NO_MREQ(MP, 1); contendWrite(MP, L); break;
case 0xa6: /* RES 4,(REG+dd)      */ { uint8_t t = contendRead(MP); CONTEND_READ_NO_MREQ(MP, 1); contendWrite(MP, t & 0xef); } break;
case 0xa7: /* LD A,RES 4,(REG+dd) */ A = contendRead(MP) & 0xef; CONTEND_READ_NO_MREQ(MP, 1); contendWrite(MP, A); break;
case 0xa8: /* LD B,RES 5,(REG+dd) */ B = contendRead(MP) & 0xdf; CONTEND_READ_NO_MREQ(MP, 1); contendWrite(MP, B); break;
case 0xa9: /* LD C,RES 5,(REG+dd) */ C = contendRead(MP) & 0xdf; CONTEND_READ_NO_MREQ(MP, 1); contendWrite(MP, C); break;
case 0xaa: /* LD D,RES 5,(REG+dd) */ D = contendRead(MP) & 0xdf; CONTEND_READ_NO_MREQ(MP, 1); contendWrite(MP, D); break;
case 0xab: /* LD E,RES 5,(REG+dd) */ E = contendRead(MP) & 0xdf; CONTEND_READ_NO_MREQ(MP, 1); contendWrite(MP, E); break;
case 0xac: /* LD H,RES 5,(REG+dd) */ H = contendRead(MP) & 0xdf; CONTEND_READ_NO_MREQ(MP, 1); contendWrite(MP, H); break;
case 0xad: /* LD L,RES 5,(REG+dd) */ L = contendRead(MP) & 0xdf; CONTEND_READ_NO_MREQ(MP, 1); contendWrite(MP, L); break;
case 0xae: /* RES 5,(REG+dd)      */ { uint8_t t = contendRead(MP); CONTEND_READ_NO_MREQ(MP, 1); contendWrite(MP, t & 0xdf); } break;
case 0xaf: /* LD A,RES 5,(REG+dd) */ A = contendRead(MP) & 0xdf; CONTEND_READ_NO_MREQ(MP, 1); contendWrite(MP, A); break;
case 0xb0: /* LD B,RES 6,(REG+dd) */ B = contendRead(MP) & 0xbf; CONTEND_READ_NO_MREQ(MP, 1); contendWrite(MP, B); break;
case 0xb1: /* LD C,RES 6,(REG+dd) */ C = contendRead(MP) & 0xbf; CONTEND_READ_NO_MREQ(MP, 1); contendWrite(MP, C); break;
case 0xb2: /* LD D,RES 6,(REG+dd) */ D = contendRead(MP) & 0xbf; CONTEND_READ_NO_MREQ(MP, 1); contendWrite(MP, D); break;
case 0xb3: /* LD E,RES 6,(REG+dd) */ E = contendRead(MP) & 0xbf; CONTEND_READ_NO_MREQ(MP, 1); contendWrite(MP, E); break;
case 0xb4: /* LD H,RES 6,(REG+dd) */ H = contendRead(MP) & 0xbf; CONTEND_READ_NO_MREQ(MP, 1); contendWrite(MP, H); break;
case 0xb5: /* LD L,RES 6,(REG+dd) */ L = contendRead(MP) & 0xbf; CONTEND_READ_NO_MREQ(MP, 1); contendWrite(MP, L); break;
case 0xb6: /* RES 6,(REG+dd)      */ { uint8_t t = contendRead(MP); CONTEND_READ_NO_MREQ(MP, 1); contendWrite(MP, t & 0xbf); } break;
case 0xb7: /* LD A,RES 6,(REG+dd) */ A = contendRead(MP) & 0xbf; CONTEND_READ_NO_MREQ(MP, 1); contendWrite(MP, A); break;
case 0xb8: /* LD B,RES 7,(REG+dd) */ B = contendRead(MP) & 0x7f; CONTEND_READ_NO_MREQ(MP, 1); contendWrite(MP, B); break;
case 0xb9: /* LD C,RES 7,(REG+dd) */ C = contendRead(MP) & 0x7f; CONTEND_READ_NO_MREQ(MP, 1); contendWrite(MP, C); break;
case 0xba: /* LD D,RES 7,(REG+dd) */ D = contendRead(MP) & 0x7f; CONTEND_READ_NO_MREQ(MP, 1); contendWrite(MP, D); break;
case 0xbb: /* LD E,RES 7,(REG+dd) */ E = contendRead(MP) & 0x7f; CONTEND_READ_NO_MREQ(MP, 1); contendWrite(MP, E); break;
case 0xbc: /* LD H,RES 7,(REG+dd) */ H = contendRead(MP) & 0x7f; CONTEND_READ_NO_MREQ(MP, 1); contendWrite(MP, H); break;
case 0xbd: /* LD L,RES 7,(REG+dd) */ L = contendRead(MP) & 0x7f; CONTEND_READ_NO_MREQ(MP, 1); contendWrite(MP, L); break;
case 0xbe: /* RES 7,(REG+dd)      */ { uint8_t t = contendRead(MP); CONTEND_READ_NO_MREQ(MP, 1); contendWrite(MP, t & 0x7f); } break;
case 0xbf: /* LD A,RES 7,(REG+dd) */ A = contendRead(MP) & 0x7f; CONTEND_READ_NO_MREQ(MP, 1); contendWrite(MP, A); break;
case 0xc0: /* LD B,SET 0,(REG+dd) */ B = contendRead(MP) | 0x01; CONTEND_READ_NO_MREQ(MP, 1); contendWrite(MP, B); break;
case 0xc1: /* LD C,SET 0,(REG+dd) */ C = contendRead(MP) | 0x01; CONTEND_READ_NO_MREQ(MP, 1); contendWrite(MP, C); break;
case 0xc2: /* LD D,SET 0,(REG+dd) */ D = contendRead(MP) | 0x01; CONTEND_READ_NO_MREQ(MP, 1); contendWrite(MP, D); break;
case 0xc3: /* LD E,SET 0,(REG+dd) */ E = contendRead(MP) | 0x01; CONTEND_READ_NO_MREQ(MP, 1); contendWrite(MP, E); break;
case 0xc4: /* LD H,SET 0,(REG+dd) */ H = contendRead(MP) | 0x01; CONTEND_READ_NO_MREQ(MP, 1); contendWrite(MP, H); break;
case 0xc5: /* LD L,SET 0,(REG+dd) */ L = contendRead(MP) | 0x01; CONTEND_READ_NO_MREQ(MP, 1); contendWrite(MP, L); break;
case 0xc6: /* SET 0,(REG+dd)      */ { uint8_t t = contendRead(MP); CONTEND_READ_NO_MREQ(MP, 1); contendWrite(MP, t | 0x01); } break;
case 0xc7: /* LD A,SET 0,(REG+dd) */ A = contendRead(MP) | 0x01; CONTEND_READ_NO_MREQ(MP, 1); contendWrite(MP, A); break;
case 0xc8: /* LD B,SET 1,(REG+dd) */ B = contendRead(MP) | 0x02; CONTEND_READ_NO_MREQ(MP, 1); contendWrite(MP, B); break;
case 0xc9: /* LD C,SET 1,(REG+dd) */ C = contendRead(MP) | 0x02; CONTEND_READ_NO_MREQ(MP, 1); contendWrite(MP, C); break;
case 0xca: /* LD D,SET 1,(REG+dd) */ D = contendRead(MP) | 0x02; CONTEND_READ_NO_MREQ(MP, 1); contendWrite(MP, D); break;
case 0xcb: /* LD E,SET 1,(REG+dd) */ E = contendRead(MP) | 0x02; CONTEND_READ_NO_MREQ(MP, 1); contendWrite(MP, E); break;
case 0xcc: /* LD H,SET 1,(REG+dd) */ H = contendRead(MP) | 0x02; CONTEND_READ_NO_MREQ(MP, 1); contendWrite(MP, H); break;
case 0xcd: /* LD L,SET 1,(REG+dd) */ L = contendRead(MP) | 0x02; CONTEND_READ_NO_MREQ(MP, 1); contendWrite(MP, L); break;
case 0xce: /* SET 1,(REG+dd)      */ { uint8_t t = contendRead(MP); CONTEND_READ_NO_MREQ(MP, 1); contendWrite(MP, t | 0x02); } break;
case 0xcf: /* LD A,SET 1,(REG+dd) */ A = contendRead(MP) | 0x02; CONTEND_READ_NO_MREQ(MP, 1); contendWrite(MP, A); break;
case 0xd0: /* LD B,SET 2,(REG+dd) */ B = contendRead(MP) | 0x04; CONTEND_READ_NO_MREQ(MP, 1); contendWrite(MP, B); break;
case 0xd1: /* LD C,SET 2,(REG+dd) */ C = contendRead(MP) | 0x04; CONTEND_READ_NO_MREQ(MP, 1); contendWrite(MP, C); break;
case 0xd2: /* LD D,SET 2,(REG+dd) */ D = contendRead(MP) | 0x04; CONTEND_READ_NO_MREQ(MP, 1); contendWrite(MP, D); break;
case 0xd3: /* LD E,SET 2,(REG+dd) */ E = contendRead(MP) | 0x04; CONTEND_READ_NO_MREQ(MP, 1); contendWrite(MP, E); break;
case 0xd4: /* LD H,SET 2,(REG+dd) */ H = contendRead(MP) | 0x04; CONTEND_READ_NO_MREQ(MP, 1); contendWrite(MP, H); break;
case 0xd5: /* LD L,SET 2,(REG+dd) */ L = contendRead(MP) | 0x04; CONTEND_READ_NO_MREQ(MP, 1); contendWrite(MP, L); break;
case 0xd6: /* SET 2,(REG+dd)      */ { uint8_t t = contendRead(MP); CONTEND_READ_NO_MREQ(MP, 1); contendWrite(MP, t | 0x04); } break;
case 0xd7: /* LD A,SET 2,(REG+dd) */ A = contendRead(MP) | 0x04; CONTEND_READ_NO_MREQ(MP, 1); contendWrite(MP, A); break;
case 0xd8: /* LD B,SET 3,(REG+dd) */ B = contendRead(MP) | 0x08; CONTEND_READ_NO_MREQ(MP, 1); contendWrite(MP, B); break;
case 0xd9: /* LD C,SET 3,(REG+dd) */ C = contendRead(MP) | 0x08; CONTEND_READ_NO_MREQ(MP, 1); contendWrite(MP, C); break;
case 0xda: /* LD D,SET 3,(REG+dd) */ D = contendRead(MP) | 0x08; CONTEND_READ_NO_MREQ(MP, 1); contendWrite(MP, D); break;
case 0xdb: /* LD E,SET 3,(REG+dd) */ E = contendRead(MP) | 0x08; CONTEND_READ_NO_MREQ(MP, 1); contendWrite(MP, E); break;
case 0xdc: /* LD H,SET 3,(REG+dd) */ H = contendRead(MP) | 0x08; CONTEND_READ_NO_MREQ(MP, 1); contendWrite(MP, H); break;
case 0xdd: /* LD L,SET 3,(REG+dd) */ L = contendRead(MP) | 0x08; CONTEND_READ_NO_MREQ(MP, 1); contendWrite(MP, L); break;
case 0xde: /* SET 3,(REG+dd)      */ { uint8_t t = contendRead(MP); CONTEND_READ_NO_MREQ(MP, 1); contendWrite(MP, t | 0x08); } break;
case 0xdf: /* LD A,SET 3,(REG+dd) */ A = contendRead(MP) | 0x08; CONTEND_READ_NO_MREQ(MP, 1); contendWrite(MP, A); break;
case 0xe0: /* LD B,SET 4,(REG+dd) */ B = contendRead(MP) | 0x10; CONTEND_READ_NO_MREQ(MP, 1); contendWrite(MP, B); break;
case 0xe1: /* LD C,SET 4,(REG+dd) */ C = contendRead(MP) | 0x10; CONTEND_READ_NO_MREQ(MP, 1); contendWrite(MP, C); break;
case 0xe2: /* LD D,SET 4,(REG+dd) */ D = contendRead(MP) | 0x10; CONTEND_READ_NO_MREQ(MP, 1); contendWrite(MP, D); break;
case 0xe3: /* LD E,SET 4,(REG+dd) */ E = contendRead(MP) | 0x10; CONTEND_READ_NO_MREQ(MP, 1); contendWrite(MP, E); break;
case 0xe4: /* LD H,SET 4,(REG+dd) */ H = contendRead(MP) | 0x10; CONTEND_READ_NO_MREQ(MP, 1); contendWrite(MP, H); break;
case 0xe5: /* LD L,SET 4,(REG+dd) */ L = contendRead(MP) | 0x10; CONTEND_READ_NO_MREQ(MP, 1); contendWrite(MP, L); break;
case 0xe6: /* SET 4,(REG+dd)      */ { uint8_t t = contendRead(MP); CONTEND_READ_NO_MREQ(MP, 1); contendWrite(MP, t | 0x10); } break;
case 0xe7: /* LD A,SET 4,(REG+dd) */ A = contendRead(MP) | 0x10; CONTEND_READ_NO_MREQ(MP, 1); contendWrite(MP, A); break;
case 0xe8: /* LD B,SET 5,(REG+dd) */ B = contendRead(MP) | 0x20; CONTEND_READ_NO_MREQ(MP, 1); contendWrite(MP, B); break;
case 0xe9: /* LD C,SET 5,(REG+dd) */ C = contendRead(MP) | 0x20; CONTEND_READ_NO_MREQ(MP, 1); contendWrite(MP, C); break;
case 0xea: /* LD D,SET 5,(REG+dd) */ D = contendRead(MP) | 0x20; CONTEND_READ_NO_MREQ(MP, 1); contendWrite(MP, D); break;
case 0xeb: /* LD E,SET 5,(REG+dd) */ E = contendRead(MP) | 0x20; CONTEND_READ_NO_MREQ(MP, 1); contendWrite(MP, E); break;
case 0xec: /* LD H,SET 5,(REG+dd) */ H = contendRead(MP) | 0x20; CONTEND_READ_NO_MREQ(MP, 1); contendWrite(MP, H); break;
case 0xed: /* LD L,SET 5,(REG+dd) */ L = contendRead(MP) | 0x20; CONTEND_READ_NO_MREQ(MP, 1); contendWrite(MP, L); break;
case 0xee: /* SET 5,(REG+dd)      */ { uint8_t t = contendRead(MP); CONTEND_READ_NO_MREQ(MP, 1); contendWrite(MP, t | 0x20); } break;
case 0xef: /* LD A,SET 5,(REG+dd) */ A = contendRead(MP) | 0x20; CONTEND_READ_NO_MREQ(MP, 1); contendWrite(MP, A); break;
case 0xf0: /* LD B,SET 6,(REG+dd) */ B = contendRead(MP) | 0x40; CONTEND_READ_NO_MREQ(MP, 1); contendWrite(MP, B); break;
case 0xf1: /* LD C,SET 6,(REG+dd) */ C = contendRead(MP) | 0x40; CONTEND_READ_NO_MREQ(MP, 1); contendWrite(MP, C); break;
case 0xf2: /* LD D,SET 6,(REG+dd) */ D = contendRead(MP) | 0x40; CONTEND_READ_NO_MREQ(MP, 1); contendWrite(MP, D); break;
case 0xf3: /* LD E,SET 6,(REG+dd) */ E = contendRead(MP) | 0x40; CONTEND_READ_NO_MREQ(MP, 1); contendWrite(MP, E); break;
case 0xf4: /* LD H,SET 6,(REG+dd) */ H = contendRead(MP) | 0x40; CONTEND_READ_NO_MREQ(MP, 1); contendWrite(MP, H); break;
case 0xf5: /* LD L,SET 6,(REG+dd) */ L = contendRead(MP) | 0x40; CONTEND_READ_NO_MREQ(MP, 1); contendWrite(MP, L); break;
case 0xf6: /* SET 6,(REG+dd)      */ { uint8_t t = contendRead(MP); CONTEND_READ_NO_MREQ(MP, 1); contendWrite(MP, t | 0x40); } break;
case 0xf7: /* LD A,SET 6,(REG+dd) */ A = contendRead(MP) | 0x40; CONTEND_READ_NO_MREQ(MP, 1); contendWrite(MP, A); break;
case 0xf8: /* LD B,SET 7,(REG+dd) */ B = contendRead(MP) | 0x80; CONTEND_READ_NO_MREQ(MP, 1); contendWrite(MP, B); break;
case 0xf9: /* LD C,SET 7,(REG+dd) */ C = contendRead(MP) | 0x80; CONTEND_READ_NO_MREQ(MP, 1); contendWrite(MP, C); break;
case 0xfa: /* LD D,SET 7,(REG+dd) */ D = contendRead(MP) | 0x80; CONTEND_READ_NO_MREQ(MP, 1); contendWrite(MP, D); break;
case 0xfb: /* LD E,SET 7,(REG+dd) */ E = contendRead(MP) | 0x80; CONTEND_READ_NO_MREQ(MP, 1); contendWrite(MP, E); break;
case 0xfc: /* LD H,SET 7,(REG+dd) */ H = contendRead(MP) | 0x80; CONTEND_READ_NO_MREQ(MP, 1); contendWrite(MP, H); break;
case 0xfd: /* LD L,SET 7,(REG+dd) */ L = contendRead(MP) | 0x80; CONTEND_READ_NO_MREQ(MP, 1); contendWrite(MP, L); break;
case 0xfe: /* SET 7,(REG+dd)      */ { uint8_t t = contendRead(MP); CONTEND_READ_NO_MREQ(MP, 1); contendWrite(MP, t | 0x80); } break;
case 0xff: /* LD A,SET 7,(REG+dd) */ A = contendRead(MP) | 0x80; CONTEND_READ_NO_MREQ(MP, 1); contendWrite(MP, A); break;