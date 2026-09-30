/*
 * func_8004A9A0 -- shop input callback installed by func_8004A570 at obj+0x2C:
 * buy / sell / confirm / cancel on the pad bits, two 7-entry switches on
 * D_8009CFD0.  Era -O2 -G8 + MASPSX_THREE_WORD_SYMBOL_STORE=1 +
 * MASPSX_DISPATCH_FOLD=jtbl_80010FF4,jtbl_80011014.
 *
 * Closed in lane exeF (2026-09-28) from exeC5's 6-word draft.  In each sell
 * case 0-2 an `asm volatile("" : : "r"(v))` between the aliased
 * `v = D_8009CFD8_k` read and the `h[k]--` is a sched barrier (retail's
 * `lw v0` ahead of `addiu v1,a0,-1`; sched's potential-hazard tie-break put
 * the load last otherwise), and the non-volatile `asm("" : : "r"(v))` after the
 * decrement carries a distinct source line so jump2 does not cross-jump the
 * identical `addiu; sh` tails.
 */
typedef struct Item {
    unsigned char pad0[0xE];
    short h[3];
    unsigned char pad14[0xC];
} Item;

typedef struct W {
    int f0;
} W;

extern Item D_800A1A00;
extern int D_800A18D8[];
extern int D_8009CFAC;
extern int D_8009CFD0;
extern int D_8009CFD8;
extern int D_8009CFD8_0 asm("D_8009CFD8");
extern int D_8009CFD8_1 asm("D_8009CFD8");
extern int D_8009CFD8_2 asm("D_8009CFD8");
extern int D_8009CFDC;
extern int D_8009CF68;
extern signed char D_800C0E20[];
extern signed char D_800C0E22[];
extern int func_80059F08(int);
extern Item *func_8005332C(int);
extern void func_8005BA78(int, int, int *, int *);
extern void func_8005267C(void);
extern void func_800526C4(void);
extern int func_8005BBE4(int);
extern int func_80052F0C(void);
extern void func_800512AC(int, int *);
extern void func_80062F1C(W *);
extern void func_800525EC(void);
extern void func_80052634(void);

int func_8004A9A0(W *self, int key)
{
    Item *rec;
    int k;
    short *p;
    int d;
    int x;
    register int v asm("$2");
    int w;
    int *p2;

    rec = func_8005332C(func_80059F08(0));
    if (key & 0x4000) {
        D_8009CFAC = 0;
        if (D_8009CFD8 < 100) {
            goto fail;
        }
        k = 0;
        switch (D_8009CFD0) {
        case 0:
            p = &D_800A1A00.h[0];
            k = *p < 999;
            *p += k;
            break;
        case 1:
            p = &D_800A1A00.h[1];
            k = *p < 999;
            *p += k;
            break;
        case 2:
            p = &D_800A1A00.h[2];
            k = *p < 999;
            *p += k;
            break;
        case 5:
        case 6:
            func_8005BA78(D_8009CFD0, D_8009CFDC, &d, 0);
            k = d > 0;
            D_8009CFDC += d;
            break;
        }
        D_8009CFD8 -= k * 100;
        if (k == 0) {
            goto fail;
        }
        func_8005267C();
        return 1;
    }
    if (key & 0x1000) {
        D_8009CFAC = 1;
        switch (D_8009CFD0) {
        case 0:
            if (rec->h[0] >= D_800A1A00.h[0]) {
                goto fail;
            }
            v = D_8009CFD8_0;
            asm volatile("" : : "r"(v));
            D_800A1A00.h[0]--;
            asm("" : : "r"(v));
            goto tail;
        case 1:
            if (rec->h[1] >= D_800A1A00.h[1]) {
                goto fail;
            }
            v = D_8009CFD8_1;
            asm volatile("" : : "r"(v));
            D_800A1A00.h[1]--;
            asm("" : : "r"(v));
            goto tail;
        case 2:
            if (rec->h[2] >= D_800A1A00.h[2]) {
                goto fail;
            }
            v = D_8009CFD8_2;
            asm volatile("" : : "r"(v));
            D_800A1A00.h[2]--;
            asm("" : : "r"(v));
            goto tail;
        case 5:
        case 6:
            p2 = &D_800A18D8[D_8009CFD0];
            {
            register int c asm("$5") = D_8009CFDC;
            if (*p2 >= c) {
                goto fail;
            }
            }
            {
            register int c asm("$5");
            int f = func_8005BBE4(D_8009CFD0);
            c = D_8009CFDC;
            D_8009CFDC = (c - f < *p2) ? D_800A18D8[D_8009CFD0] : D_8009CFDC - func_8005BBE4(D_8009CFD0);
            }
            v = D_8009CFD8;
        tail:
            D_8009CFD8 = v + 100;
            func_8005267C();
            return 1;
        default:
            return 1;
        }
    fail:
        func_800526C4();
        return 1;
    }
    if (key & 0x10000) {
        D_8009CF68 = D_8009CFD8;
        if (D_8009CFD0 < 3) {
            *rec = D_800A1A00;
            if (func_80052F0C() == 0) {
                if (rec == func_8005332C(D_800C0E20[0])) {
                    func_800512AC(2, 0);
                } else if (rec == func_8005332C(D_800C0E22[0])) {
                    func_800512AC(3, 0);
                }
            }
        } else {
            D_800A18D8[D_8009CFD0] = D_8009CFDC;
        }
        func_80062F1C(self);
        func_800525EC();
        return 1;
    }
    if (key & 0x40) {
        func_80062F1C(self);
        func_80052634();
    }
    return 1;
}
