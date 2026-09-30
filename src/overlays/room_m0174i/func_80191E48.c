/* room_m0174i (PE.IMG room m0174i chunk 2, VRAM 0x8018EFE8)
 * func_80191E48 — blob offset 0x2E60, 0x40 bytes. Profile era_o2_g0 (default);
 * LINK_EXACT at the room VMA (docs/evidence/room-lane-2026-09-23/REPORT.md).
 * Effect step: +0x12 -= 8, +0x10 += 30, +2 -= 15; done when +0x12 < 0. */

#define B(o, x) (*(unsigned char *)((char *)(o) + (x)))
#define H(o, x) (*(short *)((char *)(o) + (x)))
#define W(o, x) (*(int *)((char *)(o) + (x)))
#define P(o, x) (*(void **)((char *)(o) + (x)))
void func_80191E48(int a0, void *q, void *p)
{
    H(p, 0x12) -= 8;
    H(p, 0x10) += 30;
    H(p, 0x2) -= 15;
    if (H(p, 0x12) < 0) {
        H(p, 0x12) = 0;
        B(q, 0x1) = 2;
    }
}
