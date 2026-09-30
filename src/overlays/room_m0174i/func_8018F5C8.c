/* room_m0174i (PE.IMG room m0174i chunk 2, VRAM 0x8018EFE8)
 * func_8018F5C8 — blob offset 0x5E0, 0x3C bytes. Profile era_o2_g0 (default);
 * LINK_EXACT at the room VMA (docs/evidence/room-lane-2026-09-23/REPORT.md).
 * Clear +0, set +2 from *func_800C2B10(4). */

#define B(o, x) (*(unsigned char *)((char *)(o) + (x)))
#define H(o, x) (*(short *)((char *)(o) + (x)))
#define W(o, x) (*(int *)((char *)(o) + (x)))
#define P(o, x) (*(void **)((char *)(o) + (x)))
extern int *func_800C2B10();
void func_8018F5C8(int a0, int a1, void *p)
{
    H(p, 0x0) = 0;
    H(p, 0x2) = *func_800C2B10(4);
}
