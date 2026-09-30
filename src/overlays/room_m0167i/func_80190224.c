/* room_m0167i (PE.IMG room m0167i chunk 2, VRAM 0x8018EFE8)
 * func_80190224 — blob offset 0x123c, 0xa8 bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0022i func_8018F588; C re-targeted by symbol address
 * (docs/evidence/room_m0167i-ports-2026-09-23/REPORT.md). */

#define B(o, x) (*(unsigned char *)((char *)(o) + (x)))
#define SB(o, x) (*(signed char *)((char *)(o) + (x)))
#define H(o, x) (*(short *)((char *)(o) + (x)))
#define W(o, x) (*(int *)((char *)(o) + (x)))
#define P(o, x) (*(void **)((char *)(o) + (x)))
extern void func_80190814();
extern void func_801902CC();
void func_80190224(void *o)
{
    int v;
    void *c;
    int lo, hi;

    if (B(o, 0xB4) != 0) {
        func_80190814(P(o, 0x8), (char *)o + 0xC, 0x1F800000);
    }
    if (SB(o, 0x16) < 0 || SB(o, 0x16) == B(P(o, 0x8), 0xE)) {
        v = SB(o, 0x17);
        if (v < 0) {
            P(o, 0xC) = func_801902CC;
        } else {
            c = P(o, 0x8);
            lo = *(unsigned short *)((char *)c + 0x1A);
            hi = *(unsigned short *)((char *)c + 0x16);
            if (lo <= v && v <= hi) {
                P(o, 0xC) = func_801902CC;
            }
        }
    }
}
