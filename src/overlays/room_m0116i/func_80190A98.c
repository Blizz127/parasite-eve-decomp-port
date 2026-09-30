/* room_m0116i (PE.IMG room m0116i chunk 2, VRAM 0x8018EFE8)
 * func_80190A98 — blob offset 0x1ab0, 0x248 bytes. Profile room_m0116i_dispatch_8018F0FC (local: -O2 -G0 + dispatch fold).
 * Masked-body twin of room_m0022i func_80190A84; C re-targeted by symbol address
 * (docs/evidence/room_m0116i-ports-2026-09-23/REPORT.md). */

#define B(o, x) (*(unsigned char *)((char *)(o) + (x)))
#define H(o, x) (*(short *)((char *)(o) + (x)))
#define W(o, x) (*(int *)((char *)(o) + (x)))
#define P(o, x) (*(void **)((char *)(o) + (x)))

extern void *D_8009D20C;
extern void *D_8009D254;
extern void func_80190D10();
extern void func_80191114();

int func_80190A98(void *o, int a1, unsigned int a2, int a3, int a4, int a5)
{
    void *s = (char *)o + 0xC;

    switch (a2) {
    case 19:
        if (a1 == 0) {
            B(o, 0x3) = a3;
        } else {
            *(int *)a3 = B(o, 0x3);
        }
        break;
    case 25:
        if (a1 == 1) {
            W(s, 0x4) = a3;
            *(int *)a3 = a1;
        }
        break;
    case 4:
        W(s, 0x68) = a3;
        W(s, 0x6C) = a4;
        break;
    case 2:
        W(s, 0x70) = a3;
        break;
    case 12:
        W(s, 0x74) = a3;
        break;
    case 13:
        W(s, 0x80) = a3;
        break;
    case 14:
        H(s, 0x86) = a4;
        break;
    case 6:
        W(s, 0x40) = a3;
        W(s, 0x44) = a4;
        W(s, 0x48) = a5;
        break;
    case 23:
        a3 = a3 != 0;
        a4 = (a4 != 0) << 1;
        a5 = (a5 != 0) << 2;
        B(s, 0xD) = a3 | a4 | a5;
        break;
    case 10:
        B(s, 0xA) = a3;
        H(s, 0x8) = a4;
        P(s, 0x0) = func_80190D10;
        break;
    case 11:
        B(s, 0xB) = a3;
        B(s, 0xC) = a4;
        break;
    case 16:
        H(s, 0x88) = a3;
        break;
    case 18:
        if (B(o, 0x3) == 2) {
            P(s, 0x0) = func_80191114;
            W(s, 0x74) = a3;
            B(o, 0x3) = 3;
        }
        break;
    case 0:
        for (P(s, 0x60) = D_8009D20C; P(s, 0x60) != 0; P(s, 0x60) = P(P(s, 0x60), 0x4)) {
            if (B(P(s, 0x60), 0xC) == a3 && B(P(s, 0x60), 0xD) == a4 && !(W(P(s, 0x60), 0x98) & 0x10)) {
                break;
            }
        }
        break;
    case 17:
        W(s, 0x30) = a3;
        W(s, 0x38) = a5;
        if (a4 != -1) {
            W(s, 0x34) = a4;
        } else {
            W(s, 0x34) = W(D_8009D254, 0x2C);
        }
        W(s, 0x60) = 0;
        break;
    case 21:
        for (P(s, 0x64) = D_8009D20C; P(s, 0x64) != 0; P(s, 0x64) = P(P(s, 0x64), 0x4)) {
            if (B(P(s, 0x64), 0xC) == a3 && B(P(s, 0x64), 0xD) == a4 && !(W(P(s, 0x64), 0x98) & 0x10)) {
                break;
            }
        }
        H(s, 0x8A) = a5;
        break;
    case 22:
        W(s, 0x50) = a3;
        W(s, 0x58) = a4;
        H(s, 0x8A) = a5;
        break;
    }
    return 0;
}
