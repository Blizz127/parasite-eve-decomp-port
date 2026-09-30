/* room_m0145i (PE.IMG room m0145i chunk 2, VRAM 0x8018EFE8)
 * func_801924F8 — blob offset 0x3510, 0x64 bytes. Profile era_o2_g0 (default);
 * LINK_EXACT at the room VMA (docs/evidence/room-lane-2026-09-23/REPORT.md).
 * Behaviour init (bytes -1/-1/-1/3, fn = func_801927C8) plus ctx flag bits 0x10002 / 0x400. */

#define B(o, x) (*(unsigned char *)((char *)(o) + (x)))
#define SB(o, x) (*(signed char *)((char *)(o) + (x)))
#define H(o, x) (*(short *)((char *)(o) + (x)))
#define W(o, x) (*(int *)((char *)(o) + (x)))
#define P(o, x) (*(void **)((char *)(o) + (x)))
extern void func_801927C8();
int func_801924F8(void *o)
{
    void *c = P(o, 0x8);

    SB(o, 0x16) = -1;
    SB(o, 0x17) = -1;
    SB(o, 0x18) = -1;
    SB(o, 0x19) = 3;
    W(o, 0x10) = 0;
    H(o, 0x14) = 0;
    B(o, 0x1A) = 0;
    H(o, 0x32) = 0;
    H(o, 0x36) = 0;
    P(o, 0xC) = func_801927C8;
    W(c, 0x98) |= 0x10002;
    H(c, 0x250) |= 0x400;
    return 0;
}
