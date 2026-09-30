/* room_m0137i (PE.IMG room m0137i chunk 2, VRAM 0x8018EFE8)
 * func_8018FB58 — blob offset 0xB70, 0x84 bytes. Profile era_o2_g0 (default);
 * LINK_EXACT at the room VMA (docs/evidence/room-lane-2026-09-23/REPORT.md).
 * Effect fade step: +0x10 += 3; past the threshold +0x16 += 8, +0x15 = +0x16 >> 4, done at 8. */

#define B(o, x) (*(unsigned char *)((char *)(o) + (x)))
#define H(o, x) (*(short *)((char *)(o) + (x)))
#define W(o, x) (*(int *)((char *)(o) + (x)))
#define P(o, x) (*(void **)((char *)(o) + (x)))
extern void *func_800C2B50();
void func_8018FB58(int a0, void *q, void *p)
{
    void *r = func_800C2B50();

    H(p, 0x10) += 3;
    if (H(r, 0xC) - 30 < H(q, 0x2)) {
        B(p, 0x16) += 8;
        B(p, 0x15) = B(p, 0x16) >> 4;
        if (B(p, 0x15) == 8) {
            B(q, 0x1) = 2;
        }
    }
}
