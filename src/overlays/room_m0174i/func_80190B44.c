/* room_m0174i (PE.IMG room m0174i chunk 2, VRAM 0x8018EFE8)
 * func_80190B44 — blob offset 0x1B5C, 0x24 bytes. Profile era_o2_g0 (default);
 * LINK_EXACT at the room VMA (docs/evidence/room-lane-2026-09-23/REPORT.md).
 * Seed an effect block (+0x10 = 2772, +0x12 = 255, clear +8..+0xC and +0x14). */

#define B(o, x) (*(unsigned char *)((char *)(o) + (x)))
#define H(o, x) (*(short *)((char *)(o) + (x)))
#define W(o, x) (*(int *)((char *)(o) + (x)))
void func_80190B44(int a0, int a1, void *p)
{
    H(p, 0x10) = 2772;
    H(p, 0x12) = 255;
    H(p, 0x8) = 0;
    H(p, 0xA) = 0;
    H(p, 0xC) = 0;
    H(p, 0x14) = 0;
}
