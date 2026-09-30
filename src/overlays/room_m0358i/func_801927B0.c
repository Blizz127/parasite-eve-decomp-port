/* room_m0358i (PE.IMG room m0358i chunk 2, VRAM 0x8018EFE8)
 * func_801927B0 — blob offset 0x37C8, 0x64 bytes. Profile era_o2_g0 (default);
 * LINK_EXACT at the room VMA (docs/evidence/room-lane-2026-09-23/REPORT.md).
 * Range gate on the +0x18C sub-context; sets the behaviour to func_80192814. */

#define B(o, x) (*(unsigned char *)((char *)(o) + (x)))
#define SB(o, x) (*(signed char *)((char *)(o) + (x)))
#define H(o, x) (*(short *)((char *)(o) + (x)))
#define W(o, x) (*(int *)((char *)(o) + (x)))
#define P(o, x) (*(void **)((char *)(o) + (x)))
extern void func_80192814();
void func_801927B0(void *o)
{
    int v;
    void *c = P(P(o, 0x8), 0x18C);
    int lo, hi;

    if (SB(o, 0x16) < 0 || SB(o, 0x16) == B(c, 0xE)) {
        v = SB(o, 0x17);
        if (v < 0) {
            P(o, 0xC) = func_80192814;
        } else {
            lo = *(unsigned short *)((char *)c + 0x1A);
            hi = *(unsigned short *)((char *)c + 0x16);
            if (lo <= v && v <= hi) {
                P(o, 0xC) = func_80192814;
            }
        }
    }
}
