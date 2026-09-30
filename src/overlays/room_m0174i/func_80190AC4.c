/* room_m0174i (PE.IMG room m0174i chunk 2, VRAM 0x8018EFE8)
 * func_80190AC4 — blob offset 0x1ADC, 0x80 bytes. Profile era_o2_g0 (default);
 * LINK_EXACT at the room VMA (docs/evidence/room-lane-2026-09-23/REPORT.md).
 * Effect step keyed on +0x14 (+/-200 on +0x10), +0x12 -= 16 (floor 0), done after frame 21. */

#define B(o, x) (*(unsigned char *)((char *)(o) + (x)))
#define H(o, x) (*(short *)((char *)(o) + (x)))
#define W(o, x) (*(int *)((char *)(o) + (x)))
#define P(o, x) (*(void **)((char *)(o) + (x)))
void func_80190AC4(int a0, void *q, void *p)
{
    void *e = p;

    if (H(p, 0x14) == 1) {
        H(p, 0x10) += 200;
    }
    if (H(p, 0x14) == 0) {
        H(p, 0x10) -= 200;
    }
    H(e, 0x12) -= 16;
    if (H(e, 0x12) < 0) {
        H(e, 0x12) = 0;
    }
    if (H(q, 0x2) >= 21) {
        B(q, 0x1) = 2;
    }
}
