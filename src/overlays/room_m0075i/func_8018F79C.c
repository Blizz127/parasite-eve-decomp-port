/* room_m0075i (PE.IMG room m0075i chunk 2, VRAM 0x8018EFE8)
 * func_8018F79C — blob offset 0x7B4, 0x78 bytes. Profile era_o2_g0 (default);
 * LINK_EXACT at the room VMA (docs/evidence/room-lane-2026-09-23/REPORT.md).
 * Copy two func_800C2B50() vectors (+0x58.., +0x38..) to shorts; +0x10 = 1024. */

#define B(o, x) (*(unsigned char *)((char *)(o) + (x)))
#define H(o, x) (*(short *)((char *)(o) + (x)))
#define W(o, x) (*(int *)((char *)(o) + (x)))
#define P(o, x) (*(void **)((char *)(o) + (x)))
extern void *func_800C2B50();
void func_8018F79C(int a0, int a1, void *p)
{
    void *r = func_800C2B50();

    H(p, 0x0) = W(r, 0x58);
    H(p, 0x2) = W(r, 0x5C);
    H(p, 0x4) = W(r, 0x60);
    H(p, 0x8) = W(r, 0x38);
    H(p, 0xA) = W(r, 0x3C);
    H(p, 0xC) = W(r, 0x40);
    H(p, 0x12) = 0;
    H(p, 0x10) = 1024;
}
