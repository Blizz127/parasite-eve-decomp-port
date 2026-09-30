/* room_m0186i (PE.IMG room m0186i chunk 2, VRAM 0x8018EFE8)
 * func_8019021C — blob offset 0x1234, 0x38 bytes. Profile era_o2_g0 (default);
 * LINK_EXACT at the room VMA (docs/evidence/room-lane-2026-09-23/REPORT.md).
 * Flag +1 = 2 when func_800C2B68() == 1. */

#define B(o, x) (*(unsigned char *)((char *)(o) + (x)))
#define H(o, x) (*(short *)((char *)(o) + (x)))
#define W(o, x) (*(int *)((char *)(o) + (x)))
#define P(o, x) (*(void **)((char *)(o) + (x)))
extern int func_800C2B68();
void func_8019021C(int a0, void *q)
{
    if (func_800C2B68() == 1) {
        B(q, 0x1) = 2;
    }
}
