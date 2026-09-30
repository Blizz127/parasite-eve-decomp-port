/* room_m0167i (PE.IMG room m0167i chunk 2, VRAM 0x8018EFE8)
 * func_8018F318 — blob offset 0x330, 0x84 bytes. Profile era_o2_g0 (default);
 * LINK_EXACT at the room VMA (docs/evidence/room-lane-2026-09-23/REPORT.md).
 * State 4; if func_800C6CE0(o) >= 2 clear bits 24-29 of the ctx word and set its +0x18 byte to 4. */

#define P(o, x) (*(void **)((char *)(o) + (x)))
#define W(o, x) (*(int *)((char *)(o) + (x)))
extern unsigned int func_800C6CE0();
int func_8018F318(unsigned char *o)
{
    int s = 4;

    o[0] = s;
    if (func_800C6CE0(o) >= 2) {
        W(P(P(o, 0x8), 0x0), 0x0) &= 0xC0FFFFFF;
        *(unsigned char *)P(P(P(o, 0x8), 0x0), 0x18) = s;
    }
    return 0;
}
