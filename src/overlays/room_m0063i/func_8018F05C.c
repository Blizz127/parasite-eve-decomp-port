/* room_m0063i (PE.IMG room m0063i chunk 2, VRAM 0x8018EFE8)
 * func_8018F05C — blob offset 0x74, 0x74 bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0358i func_80192550; C re-targeted by symbol address
 * (docs/evidence/room_m0063i-ports-2026-09-23/REPORT.md). */

#define B(o, x) (*(unsigned char *)((char *)(o) + (x)))
#define SB(o, x) (*(signed char *)((char *)(o) + (x)))
#define H(o, x) (*(short *)((char *)(o) + (x)))
#define W(o, x) (*(int *)((char *)(o) + (x)))
#define P(o, x) (*(void **)((char *)(o) + (x)))
extern void func_8018F320();
int func_8018F05C(void *o)
{
    B(o, 0x3) = 1;
    SB(o, 0x16) = -1;
    SB(o, 0x17) = -1;
    SB(o, 0x18) = -1;
    SB(o, 0x19) = 3;
    W(o, 0x88) = 1024;
    P(o, 0xC) = func_8018F320;
    W(o, 0x10) = 0;
    H(o, 0x14) = 0;
    B(o, 0x1A) = 0;
    H(o, 0x94) = 0;
    W(o, 0x80) = 0;
    W(o, 0x84) = 0;
    W(o, 0x7C) = 0;
    W(o, 0x3C) = 0;
    W(o, 0x40) = 0;
    W(o, 0x44) = 0;
    H(o, 0x6C) = 0;
    H(o, 0x6E) = 0;
    H(o, 0x70) = 0;
    B(o, 0x19) = 0;
    return 0;
}
