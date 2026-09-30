/* room_m0174i (PE.IMG room m0174i chunk 2, VRAM 0x8018EFE8)
 * func_80191200 — blob offset 0x2218, 0x50 bytes. Profile era_o2_g0 (default);
 * LINK_EXACT at the room VMA (docs/evidence/room-lane-2026-09-23/REPORT.md).
 * Effect step: +0x10 += 100, +0x12 += +0x16, +0x16 += 50, +0x14 -= 16; done below 0. */

#define B(o, x) (*(unsigned char *)((char *)(o) + (x)))
#define H(o, x) (*(short *)((char *)(o) + (x)))
#define W(o, x) (*(int *)((char *)(o) + (x)))
#define P(o, x) (*(void **)((char *)(o) + (x)))
void func_80191200(int a0, void *q, void *p)
{
    H(p, 0x10) += 100;
    H(p, 0x12) += H(p, 0x16);
    H(p, 0x16) += 50;
    H(p, 0x14) -= 16;
    if (H(p, 0x14) < 0) {
        B(q, 0x1) = 2;
        H(p, 0x14) = 0;
    }
}
