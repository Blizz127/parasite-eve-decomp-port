/* room_m0421i (PE.IMG room m0421i chunk 2, VRAM 0x8018EFE8)
 * func_8018FB44 — blob offset 0xB5C, 0x64 bytes. Profile era_o2_g0 (default);
 * LINK_EXACT at the room VMA (docs/evidence/room-lane-2026-09-23/REPORT.md).
 * Room timer/effect helper (see REPORT). */

#define B(o, x) (*(unsigned char *)((char *)(o) + (x)))
#define H(o, x) (*(short *)((char *)(o) + (x)))
#define W(o, x) (*(int *)((char *)(o) + (x)))
#define P(o, x) (*(void **)((char *)(o) + (x)))
void func_8018FB44(int a0, void *q, void *p)
{
    H(p, 0x10) += 15;
    if (H(q, 0x2) < 16) {
        H(p, 0x12) += 8;
    } else {
        H(p, 0x12) -= 4;
    }
    H(p, 0xA) += 100;
    if (H(p, 0x12) <= 0) {
        B(q, 0x1) = 2;
    }
}
