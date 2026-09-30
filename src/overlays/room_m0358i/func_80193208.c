/* room_m0358i (PE.IMG room m0358i chunk 2, VRAM 0x8018EFE8)
 * func_80193208 — blob offset 0x4220, 0x48 bytes. Profile era_o2_g0 (default);
 * LINK_EXACT at the room VMA (docs/evidence/room-lane-2026-09-23/REPORT.md).
 * Call func_80193250(o) once the ctx +0x18C counter reaches its byte limit - 1. */

#define B(o, x) (*(unsigned char *)((char *)(o) + (x)))
#define H(o, x) (*(short *)((char *)(o) + (x)))
#define W(o, x) (*(int *)((char *)(o) + (x)))
#define P(o, x) (*(void **)((char *)(o) + (x)))
extern void func_80193250();
void func_80193208(void *o)
{
    void *c = P(P(o, 0x8), 0x18C);

    if (*(unsigned short *)((char *)c + 0x1A) >= (unsigned int)(B(c, 0xF) - 1)) {
        func_80193250(o);
    }
}
