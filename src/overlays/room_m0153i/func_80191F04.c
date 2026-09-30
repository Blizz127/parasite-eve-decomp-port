/* room_m0153i (PE.IMG room m0153i chunk 2, VRAM 0x8018EFE8)
 * func_80191F04 — blob offset 0x2f1c, 0x9c bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0022i func_801909E8; C re-targeted by symbol address
 * (docs/evidence/room_m0153i-ports-2026-09-23/REPORT.md). */

#define B(o, x) (*(unsigned char *)((char *)(o) + (x)))
#define SB(o, x) (*(signed char *)((char *)(o) + (x)))
#define H(o, x) (*(short *)((char *)(o) + (x)))
#define W(o, x) (*(int *)((char *)(o) + (x)))
#define P(o, x) (*(void **)((char *)(o) + (x)))
extern void func_80192288();
int func_80191F04(void *o)
{
    void *c;

    B(o, 0x3) = 1;
    SB(o, 0x17) = -1;
    SB(o, 0x16) = -1;
    SB(o, 0x19) = 7;
    W(o, 0x8C) = 0x10000;
    c = P(o, 0x8);
    W(o, 0x10) = 0;
    H(o, 0x14) = 0;
    B(o, 0x1A) = 0;
    W(o, 0x74) = 0;
    W(o, 0x78) = 0;
    W(o, 0x7C) = 0;
    W(o, 0x80) = 0;
    H(o, 0x92) = 0;
    W(o, 0x4C) = 0;
    W(o, 0x50) = 0;
    W(o, 0x54) = 0;
    H(o, 0x94) = 0;
    W(o, 0x6C) = 0;
    P(o, 0xC) = func_80192288;
    W(o, 0x70) = 0;
    H(o, 0x96) = 0;
    W(c, 0x68) = 0;
    W(c, 0x6C) = 0;
    W(c, 0x70) = 0;
    W(c, 0x88) = 0;
    W(c, 0x8C) = 0;
    W(c, 0x90) = 0;
    W(c, 0x78) = 0;
    W(c, 0x7C) = 0;
    W(c, 0x80) = 0;
    return 0;
}
