/* room_m0063i (PE.IMG room m0063i chunk 2, VRAM 0x8018EFE8)
 * func_8018F2BC — blob offset 0x2d4, 0x64 bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0358i func_801927B0; C re-targeted by symbol address
 * (docs/evidence/room_m0063i-ports-2026-09-23/REPORT.md). */

#define B(o, x) (*(unsigned char *)((char *)(o) + (x)))
#define SB(o, x) (*(signed char *)((char *)(o) + (x)))
#define H(o, x) (*(short *)((char *)(o) + (x)))
#define W(o, x) (*(int *)((char *)(o) + (x)))
#define P(o, x) (*(void **)((char *)(o) + (x)))
extern void func_8018F320();
void func_8018F2BC(void *o)
{
    int v;
    void *c = P(P(o, 0x8), 0x18C);
    int lo, hi;

    if (SB(o, 0x16) < 0 || SB(o, 0x16) == B(c, 0xE)) {
        v = SB(o, 0x17);
        if (v < 0) {
            P(o, 0xC) = func_8018F320;
        } else {
            lo = *(unsigned short *)((char *)c + 0x1A);
            hi = *(unsigned short *)((char *)c + 0x16);
            if (lo <= v && v <= hi) {
                P(o, 0xC) = func_8018F320;
            }
        }
    }
}
