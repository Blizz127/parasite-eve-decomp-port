/* room_m0063i — func_8018F0D0, blob offset 0xE8, 0x178 bytes. Flags -O2 -G0 + MASPSX_THREE_WORD_SYMBOL_STORE + MASPSX_DISPATCH_FOLD=jtbl_8018EFF4;
 * disc1_preflight --deep PASS at the target VMA (lane ovl9 2026-09-28).
 * Object message handler. Lever (case 10): fn-pointer local f kept live into the a3 != -1 arm by asm("" : : "r"(f)) — the sw does not kill f, so sched1 leaves it last, la gets $v1 and dbr puts the sw in the beq slot (5 -> 0). */

#define B(o, x) (*(unsigned char *)((char *)(o) + (x)))
#define H(o, x) (*(short *)((char *)(o) + (x)))
#define W(o, x) (*(int *)((char *)(o) + (x)))
#define P(o, x) (*(void **)((char *)(o) + (x)))

extern void *D_8009D20C;
extern void *D_8009D254;
extern void func_8018F2BC();

int func_8018F0D0(void *o, int a1, unsigned int a2, int a3, int a4, int a5)
{
    void *s = (char *)o + 0xC;
    void *f;

    switch (a2) {
    case 19:
        if (a1 == 0) {
            B(o, 0x3) = a3;
        } else {
            *(int *)a3 = B(o, 0x3);
        }
        break;
    case 0:
        for (P(s, 0x70) = D_8009D20C; P(s, 0x70) != 0; P(s, 0x70) = P(P(s, 0x70), 0x4)) {
            if (B(P(s, 0x70), 0xC) == a3 && B(P(s, 0x70), 0xD) == a4 && !(W(P(s, 0x70), 0x98) & 0x10)) {
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
        W(s, 0x70) = 0;
        break;
    case 4:
        W(s, 0x74) = a3;
        break;
    case 2:
        W(s, 0x78) = a3;
        break;
    case 3:
        W(s, 0x7C) = a3;
        break;
    case 23:
        a3 = a3 != 0;
        a4 = (a4 != 0) << 1;
        a5 = (a5 != 0) << 2;
        B(s, 0xD) = a3 | a4 | a5;
        break;
    case 10:
        f = func_8018F2BC;
        B(s, 0xA) = a3;
        H(s, 0x8) = a4;
        P(s, 0x0) = f;
        if (a3 != -1) {
            asm("" : : "r"(f));
        }
        break;
    case 11:
        B(s, 0xB) = a3;
        break;
    case 6:
        H(s, 0x60) = a3 >> 16;
        H(s, 0x62) = a4 >> 16;
        H(s, 0x64) = a5 >> 16;
        break;
    }
    return 0;
}
