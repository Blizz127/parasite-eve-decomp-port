/* room_m0399i (PE.IMG room m0399i chunk 2, VRAM 0x8018EFE8)
 * func_80193560 — blob offset 0x4578, 0x114 bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0145i func_8019255C; C re-targeted by symbol address
 * (docs/evidence/room_m0399i-ports-2026-09-23/REPORT.md). */

#define B(o, x) (*(unsigned char *)((char *)(o) + (x)))
#define H(o, x) (*(short *)((char *)(o) + (x)))
#define W(o, x) (*(int *)((char *)(o) + (x)))
#define P(o, x) (*(void **)((char *)(o) + (x)))
extern void *D_8009D20C;
extern void func_80193CC0();
int func_80193560(void *o, int a1, unsigned int a2, int a3, int a4, int a5)
{
    void *s = (char *)o + 0xC;

    switch (a2) {
    case 25:
        if (a1 == 1) {
            W(o, 0x10) = a3;
            *(int *)a3 = a1;
        }
        break;
    case 4:
        W(o, 0x20) = a3;
        break;
    case 0:
        for (P(s, 0x10) = D_8009D20C; P(s, 0x10) != 0; P(s, 0x10) = P(P(s, 0x10), 0x4)) {
            if (B(P(s, 0x10), 0xC) == a3 && B(P(s, 0x10), 0xD) == a4 && !(W(P(s, 0x10), 0x98) & 0x10)) {
                break;
            }
        }
        if (P(s, 0x10) == 0) {
            func_80193CC0();
        }
        break;
    case 12:
        W(o, 0x24) = a3;
        W(o, 0x2C) = a4;
        H(o, 0x34) = a5;
        break;
    }
    return 0;
}
