/* room_m0174i (PE.IMG room m0174i chunk 2, VRAM 0x8018EFE8)
 * func_80192EC8 — blob offset 0x3EE0, 0x6C bytes. Profile era_o2_g0 (default);
 * LINK_EXACT at the room VMA (docs/evidence/room-lane-2026-09-23/REPORT.md).
 * Effect step: +0x30 -= 150 (floor 400); after frame 8 +0x34 -= 16, done below 0. */

#define B(o, x) (*(unsigned char *)((char *)(o) + (x)))
#define H(o, x) (*(short *)((char *)(o) + (x)))
#define W(o, x) (*(int *)((char *)(o) + (x)))
#define P(o, x) (*(void **)((char *)(o) + (x)))
void func_80192EC8(int a0, void *q, void *p)
{
    void *e = p;

    H(p, 0x30) -= 150;
    if (H(p, 0x30) < 400) {
        H(p, 0x30) = 400;
    }
    if (H(q, 0x2) >= 8) {
        H(e, 0x34) -= 16;
        if (H(e, 0x34) < 0) {
            H(e, 0x34) = 0;
            B(q, 0x1) = 2;
        }
    }
}
