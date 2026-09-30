/* room_m0273i (PE.IMG room m0273i chunk 2, VRAM 0x8018EFE8)
 * func_8018F5A4 — blob offset 0x5bc, 0xa8 bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0022i func_8018F588; C re-targeted by symbol address
 * (docs/evidence/room_m0273i-ports-2026-09-23/REPORT.md). */

#define B(o, x) (*(unsigned char *)((char *)(o) + (x)))
#define SB(o, x) (*(signed char *)((char *)(o) + (x)))
#define H(o, x) (*(short *)((char *)(o) + (x)))
#define W(o, x) (*(int *)((char *)(o) + (x)))
#define P(o, x) (*(void **)((char *)(o) + (x)))
extern void func_8018FB94();
extern void func_8018F64C();
void func_8018F5A4(void *o)
{
    int v;
    void *c;
    int lo, hi;

    if (B(o, 0xB4) != 0) {
        func_8018FB94(P(o, 0x8), (char *)o + 0xC, 0x1F800000);
    }
    if (SB(o, 0x16) < 0 || SB(o, 0x16) == B(P(o, 0x8), 0xE)) {
        v = SB(o, 0x17);
        if (v < 0) {
            P(o, 0xC) = func_8018F64C;
        } else {
            c = P(o, 0x8);
            lo = *(unsigned short *)((char *)c + 0x1A);
            hi = *(unsigned short *)((char *)c + 0x16);
            if (lo <= v && v <= hi) {
                P(o, 0xC) = func_8018F64C;
            }
        }
    }
}
