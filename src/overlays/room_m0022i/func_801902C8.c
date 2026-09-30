/* room_m0022i (PE.IMG room m0022i chunk 2, VRAM 0x8018EFE8)
 * func_801902C8 — blob offset 0x12e0, 0xa8 bytes. Profile era_o2_g0 (default);
 * LINK_EXACT at the room VMA (docs/evidence/room_m0022i-batch-2026-09-23/REPORT.md). */

#define B(o, x) (*(unsigned char *)((char *)(o) + (x)))
#define SB(o, x) (*(signed char *)((char *)(o) + (x)))
#define H(o, x) (*(short *)((char *)(o) + (x)))
#define W(o, x) (*(int *)((char *)(o) + (x)))
#define P(o, x) (*(void **)((char *)(o) + (x)))
extern void func_801908FC();
extern void func_80190370();
void func_801902C8(void *o)
{
    int v;
    void *c;
    int lo, hi;
    void *s = (char *)o + 0xC;

    if (H(o, 0x80) != 0) {
        func_801908FC(P(o, 0x8), s);
    }
    if (SB(o, 0x16) < 0 || SB(o, 0x16) == B(P(o, 0x8), 0xE)) {
        v = SB(o, 0x17);
        if (v < 0) {
            P(o, 0xC) = func_80190370;
        } else {
            c = P(o, 0x8);
            lo = *(unsigned short *)((char *)c + 0x1A);
            hi = *(unsigned short *)((char *)c + 0x16);
            if (lo <= v && v <= hi) {
                P(o, 0xC) = func_80190370;
            }
        }
    }
}
