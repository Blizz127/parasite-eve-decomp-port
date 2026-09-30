/* room_m0203i (PE.IMG room m0203i chunk 2, VRAM 0x8018EFE8)
 * func_8019031C — blob offset 0x1334, 0xa8 bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0022i func_801902C8; C re-targeted by symbol address
 * (docs/evidence/room_m0203i-ports-2026-09-23/REPORT.md). */

#define B(o, x) (*(unsigned char *)((char *)(o) + (x)))
#define SB(o, x) (*(signed char *)((char *)(o) + (x)))
#define H(o, x) (*(short *)((char *)(o) + (x)))
#define W(o, x) (*(int *)((char *)(o) + (x)))
#define P(o, x) (*(void **)((char *)(o) + (x)))
extern void func_80190950();
extern void func_801903C4();
void func_8019031C(void *o)
{
    int v;
    void *c;
    int lo, hi;
    void *s = (char *)o + 0xC;

    if (H(o, 0x80) != 0) {
        func_80190950(P(o, 0x8), s);
    }
    if (SB(o, 0x16) < 0 || SB(o, 0x16) == B(P(o, 0x8), 0xE)) {
        v = SB(o, 0x17);
        if (v < 0) {
            P(o, 0xC) = func_801903C4;
        } else {
            c = P(o, 0x8);
            lo = *(unsigned short *)((char *)c + 0x1A);
            hi = *(unsigned short *)((char *)c + 0x16);
            if (lo <= v && v <= hi) {
                P(o, 0xC) = func_801903C4;
            }
        }
    }
}
