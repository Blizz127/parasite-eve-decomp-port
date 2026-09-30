/* room_m0392i (PE.IMG room m0392i chunk 2, VRAM 0x8018EFE8)
 * func_801903DC — blob offset 0x13f4, 0x284 bytes. Profile room_m0392i_dispatch_8018F02C (local: -O2 -G0 + dispatch fold).
 * Masked-body twin of room_m0022i func_8018F274; C re-targeted by symbol address
 * (docs/evidence/room_m0392i-ports-2026-09-23/REPORT.md). */

#define B(o, x) (*(unsigned char *)((char *)(o) + (x)))
#define H(o, x) (*(short *)((char *)(o) + (x)))
#define W(o, x) (*(int *)((char *)(o) + (x)))
#define P(o, x) (*(void **)((char *)(o) + (x)))

extern void *D_8009D20C;
extern void *D_8009D254;
extern void func_801906F0();

int func_801903DC(void *o, int a1, unsigned int a2, int a3, int a4, int a5)
{
    void *s = (char *)o + 0xC;

    switch (a2) {
    case 19:
        if (a1 == 0) {
            B(o, 0x3) = a3;
        } else {
            *(int *)a3 = B(o, 0x3);
            *(int *)a4 = H(s, 0x9A);
        }
        break;
    case 25:
        if (a1 == 1) {
            W(s, 0x4) = a3;
            *(int *)a3 = a1;
        }
        break;
    case 0:
        for (P(s, 0x80) = D_8009D20C; P(s, 0x80) != 0; P(s, 0x80) = P(P(s, 0x80), 0x4)) {
            if (B(P(s, 0x80), 0xC) == a3 && B(P(s, 0x80), 0xD) == a4 && !(W(P(s, 0x80), 0x98) & 0x10)) {
                break;
            }
        }
        break;
    case 17:
        W(s, 0x40) = a3;
        W(s, 0x48) = a5;
        if (a4 == -1) {
            W(s, 0x44) = W(D_8009D254, 0x2C);
        } else {
            W(s, 0x44) = a4;
        }
        W(s, 0x80) = 0;
        break;
    case 26:
        W(s, 0x40) = a3;
        W(s, 0x44) = W(P(o, 0x8), 0x2C);
        W(s, 0x48) = a4;
        W(s, 0x4C) = a5;
        W(s, 0x80) = 0;
        break;
    case 4:
        W(s, 0x88) = a3;
        break;
    case 2:
        W(s, 0x8C) = a3;
        break;
    case 3:
        W(s, 0x90) = a3;
        H(s, 0xA6) = a4;
        if (a5 > 0) {
            H(s, 0xA4) = a5;
        }
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
        B(s, 0xA8) = a5;
        P(s, 0x0) = func_801906F0;
        break;
    case 11:
        B(s, 0xB) = a3;
        B(s, 0xC) = a4;
        break;
    case 6:
        H(s, 0x70) = a3 >> 16;
        H(s, 0x72) = a4 >> 16;
        H(s, 0x74) = a5 >> 16;
        break;
    case 20:
        H(s, 0x9C) = a3;
        H(s, 0x9E) = a4;
        H(s, 0xA0) = a5;
        break;
    case 21:
        for (P(s, 0x84) = D_8009D20C; P(s, 0x84) != 0; P(s, 0x84) = P(P(s, 0x84), 0x4)) {
            if (B(P(s, 0x84), 0xC) == a3 && B(P(s, 0x84), 0xD) == a4 && !(W(P(s, 0x84), 0x98) & 0x10)) {
                break;
            }
        }
        H(s, 0xA2) = a5 & 0xFFF;
        B(s, 0xA9) = (unsigned int)a5 >> 31;
        break;
    case 22:
        W(s, 0x60) = a3;
        W(s, 0x68) = a4;
        H(s, 0xA2) = a5 & 0xFFF;
        B(s, 0xA9) = (unsigned int)a5 >> 31;
        break;
    case 30:
        B(s, 0xAA) = a3;
        break;
    case 31:
        B(s, 0xAB) = a3;
        B(s, 0xAC) = a4;
        break;
    }
    return 0;
}
