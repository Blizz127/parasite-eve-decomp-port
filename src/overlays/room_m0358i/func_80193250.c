/* room_m0358i (PE.IMG room m0358i chunk 2, VRAM 0x8018EFE8)
 * func_80193250 — blob offset 0x4268, 0x78 bytes. Profile era_o2_g0 (default);
 * LINK_EXACT at the room VMA (docs/evidence/room-lane-2026-09-23/REPORT.md).
 * State 4: clear ctx flag bits 0x10000 / 0x400 and set the +0x18C sub-actor byte to 4. */

#define B(o, x) (*(unsigned char *)((char *)(o) + (x)))
#define SB(o, x) (*(signed char *)((char *)(o) + (x)))
#define H(o, x) (*(short *)((char *)(o) + (x)))
#define W(o, x) (*(int *)((char *)(o) + (x)))
#define P(o, x) (*(void **)((char *)(o) + (x)))
int func_80193250(void *o)
{
    void *s;

    B(o, 0x0) = 4;
    B(o, 0x3) = 0;
    W(P(o, 0x8), 0x98) &= 0xFFFEFFFF;
    H(P(o, 0x8), 0x250) &= 0xFBFF;
    s = P(P(o, 0x8), 0x18C);
    if (s != 0 && P(s, 0x0) != 0) {
        *(unsigned char *)P(P(s, 0x0), 0x18) = 4;
    }
    return 0;
}
