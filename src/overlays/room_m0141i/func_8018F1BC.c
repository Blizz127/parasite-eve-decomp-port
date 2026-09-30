/* room_m0141i (PE.IMG room m0141i chunk 2, VRAM 0x8018EFE8)
 * func_8018F1BC — blob offset 0x1D4, 0x60 bytes. Profile era_o2_g0 (default);
 * LINK_EXACT at the room VMA (docs/evidence/room-lane-2026-09-23/REPORT.md).
 * State 4; if func_800C6CE0(o) >= 2 clear bits 24-29 of the ctx word. */

#define B(o, x) (*(unsigned char *)((char *)(o) + (x)))
#define H(o, x) (*(short *)((char *)(o) + (x)))
#define W(o, x) (*(int *)((char *)(o) + (x)))
#define P(o, x) (*(void **)((char *)(o) + (x)))
extern unsigned int func_800C6CE0();
int func_8018F1BC(unsigned char *o)
{
    o[0] = 4;
    if (func_800C6CE0(o) >= 2) {
        W(P(P(o, 0x8), 0x0), 0x0) &= 0xC0FFFFFF;
    }
    return 0;
}
