/* room_m0174i (PE.IMG room m0174i chunk 2, VRAM 0x8018EFE8)
 * func_80190D4C — blob offset 0x1D64, 0x58 bytes. Profile era_o2_g0 (default);
 * LINK_EXACT at the room VMA (docs/evidence/room-lane-2026-09-23/REPORT.md).
 * Effect step: +0x10 += 200, +0x12 -= 16 (floor 0), +0x14++, done at 8. */

#define B(o, x) (*(unsigned char *)((char *)(o) + (x)))
#define H(o, x) (*(short *)((char *)(o) + (x)))
#define W(o, x) (*(int *)((char *)(o) + (x)))
void func_80190D4C(int a0, void *q, void *p)
{
    H(p, 0x10) += 200;
    H(p, 0x12) -= 16;
    H(p, 0x14) += 1;
    if (H(p, 0x14) >= 8) {
        B(q, 0x1) = 2;
    }
    if (H(p, 0x12) < 0) {
        H(p, 0x12) = 0;
    }
}
