/* room_m0269i (PE.IMG room m0269i chunk 2, VRAM 0x8018EFE8)
 * func_8018F160 — blob offset 0x178, 0x2C bytes. Profile era_o2_g0 (default);
 * LINK_EXACT at the room VMA (docs/evidence/room-lane-2026-09-23/REPORT.md).
 * Set the behaviour to func_8018F18C when the ctx mode byte is 16. */

#define B(o, x) (*(unsigned char *)((char *)(o) + (x)))
#define H(o, x) (*(short *)((char *)(o) + (x)))
#define W(o, x) (*(int *)((char *)(o) + (x)))
#define P(o, x) (*(void **)((char *)(o) + (x)))
extern void func_8018F18C();
void func_8018F160(void *o)
{
    if (B(P(o, 0x8), 0xE) == 16) {
        P(o, 0xC) = func_8018F18C;
    }
}
