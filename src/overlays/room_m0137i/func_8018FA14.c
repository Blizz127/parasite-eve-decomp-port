/* room_m0137i (PE.IMG room m0137i chunk 2, VRAM 0x8018EFE8)
 * func_8018FA14 — blob offset 0xA2C, 0x18 bytes. Profile era_o2_g0 (default);
 * LINK_EXACT at the room VMA (docs/evidence/room-lane-2026-09-23/REPORT.md).
 * Seed +0x10 = 600, +0x12 = 128, clear +0x15. */

#define B(o, x) (*(unsigned char *)((char *)(o) + (x)))
#define H(o, x) (*(short *)((char *)(o) + (x)))
#define W(o, x) (*(int *)((char *)(o) + (x)))
#define P(o, x) (*(void **)((char *)(o) + (x)))
void func_8018FA14(int a0, int a1, void *p)
{
    H(p, 0x10) = 600;
    H(p, 0x12) = 128;
    B(p, 0x15) = 0;
}
