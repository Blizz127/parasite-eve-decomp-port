/* room_m0273i — func_80195E10, blob offset 0x6E28, 0x168 bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl13 2026-09-28).
 * k==4 sprite (li-before): pinned four $3 + asm volatile use after t, input-only asm("" : : "r"(s)) after the table read, u/k pinned $5/$4; rodata C4 local. */

typedef struct { unsigned char b[4]; } C4;
extern C4 D_8018F1E4;
extern int D_800E27EC;
extern unsigned short D_800F336C;
extern unsigned short D_800E1204[];
extern int D_800F3428;
extern short D_800966EC[];
extern unsigned short func_80077AA4();
extern void func_800CEE20();

int func_80195E10(int a0, short *a1)
{
    C4 c = D_8018F1E4;
    int t;
    int s;
    register int k asm("$4");
    register int u asm("$5");
    register int four asm("$3");

    if (a0 == 1) {
        if (D_800E27EC >= 0x10) {
            return 1;
        }
        a1[1] -= a1[3];
        a1[3]++;
    } else if (a0 == 2) {
        t = D_800E27EC - 1;
        four = 4;
        asm volatile("" : : "r"(four));
        s = *(short *)((char *)D_800966EC + ((t << 8) & 0x3F00)) * 2;
        asm("" : : "r"(s));
        k = D_800F336C;
        u = D_800E1204[k];
        if (k == four && D_800F3428 != 0) {
            u += 9;
        } else {
            u += 5;
        }
        func_800CEE20(a1, 0, s, s, 0x66, func_80077AA4(0, u), 1,
                      (short)*(int *)((char *)D_800966EC + ((t << 9) & 0x3E00)) >> 6, &c);
    }
    return 0;
}
