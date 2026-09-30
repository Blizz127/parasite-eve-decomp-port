/* room_m0141i (PE.IMG room m0141i chunk 2, VRAM 0x8018EFE8)
 * func_8018F950 — blob offset 0x968, 0x54 bytes. Profile era_o2_g0 (default);
 * LINK_EXACT at the room VMA (docs/evidence/room-lane-2026-09-23/REPORT.md).
 * Copy the func_800C2B50() position (+0x18..+0x20) to shorts, +8 = 1024. */

#define B(o, x) (*(unsigned char *)((char *)(o) + (x)))
#define H(o, x) (*(short *)((char *)(o) + (x)))
#define W(o, x) (*(int *)((char *)(o) + (x)))
#define P(o, x) (*(void **)((char *)(o) + (x)))
extern void *func_800C2B50();
void func_8018F950(int a0, int a1, void *p)
{
    void *r = func_800C2B50();

    H(p, 0x0) = W(r, 0x18);
    H(p, 0x2) = W(r, 0x1C);
    H(p, 0x4) = W(r, 0x20);
    H(p, 0xA) = 0;
    H(p, 0x8) = 1024;
}
