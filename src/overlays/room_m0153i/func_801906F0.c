/* room_m0153i (PE.IMG room m0153i chunk 2, VRAM 0x8018EFE8)
 * func_801906F0 — blob offset 0x1708, 0xa0 bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0022i func_8018F1D4; C re-targeted by symbol address
 * (docs/evidence/room_m0153i-ports-2026-09-23/REPORT.md). */

#define B(o, x) (*(unsigned char *)((char *)(o) + (x)))
#define SB(o, x) (*(signed char *)((char *)(o) + (x)))
#define H(o, x) (*(short *)((char *)(o) + (x)))
#define W(o, x) (*(int *)((char *)(o) + (x)))
#define P(o, x) (*(void **)((char *)(o) + (x)))
extern void func_80190B4C();
int func_801906F0(void *o)
{
    B(o, 0x3) = 1;
    SB(o, 0x16) = -1;
    SB(o, 0x17) = -1;
    SB(o, 0x18) = -1;
    SB(o, 0x19) = 3;
    W(o, 0x9C) = 0x400;
    H(o, 0xB0) = 0x100;
    P(o, 0xC) = func_80190B4C;
    W(o, 0x10) = 0;
    H(o, 0x14) = 0;
    B(o, 0x1A) = 0;
    W(o, 0x94) = 0;
    W(o, 0x98) = 0;
    W(o, 0x8C) = 0;
    W(o, 0x3C) = 0;
    W(o, 0x40) = 0;
    W(o, 0x44) = 0;
    W(o, 0xA4) = 0;
    H(o, 0xA8) = 0;
    H(o, 0xAA) = 0;
    H(o, 0xAC) = 0;
    H(o, 0x7C) = 0;
    H(o, 0x7E) = 0;
    H(o, 0x80) = 0;
    W(o, 0x90) = 0;
    H(o, 0xAE) = 0;
    W(o, 0x58) = 0;
    H(o, 0xB2) = 0;
    B(o, 0xB6) = 0;
    B(o, 0xB7) = 0;
    B(o, 0xB8) = 0;
    return 0;
}
