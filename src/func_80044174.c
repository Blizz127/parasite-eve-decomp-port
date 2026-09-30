/*
 * func_80044174 — VRAM 0x80044174 / file 0x34974 / size 0x100 (64 words).
 * Allocates the help/menu node pair: 62D2C(0x1B) then 62D2C(1, arg0) with
 * func_800447F0 in n1+0x30, 6322C(1, n2, n2) with func_80044444 in n2+0x2C
 * and func_8004F8D0 in n3+0x30, installs func_80057C54/func_80050260 at
 * n3+0x84/+0x88, runs func_80055760 and func_800647D0(n3, func_80052F70()),
 * then stores -1 into gp+0x224/+0x21C and, if gp+0x228 is non-zero,
 * decomposes v-1 into n3+0x44/+0x48/+0x5C.
 * Build: era -O2 -G8.
 */

typedef struct N N;
struct N {
    char pad00[0x2C];             /* 0x00..0x2B */
    void *f2C;                    /* +0x2C */
    void *f30;                    /* +0x30 */
    char pad34[0x10];             /* 0x34..0x43 */
    int f44;                      /* +0x44 */
    int f48;                      /* +0x48 */
    char pad4C[0x10];             /* 0x4C..0x5B */
    int f5C;                      /* +0x5C */
    char pad60[0x24];             /* 0x60..0x83 */
    void *f84;                    /* +0x84 */
    void *f88;                    /* +0x88 */
};

extern void func_800447F0(void);
extern void func_80044444(void);
extern void func_8004F8D0(void);
extern void func_80057C54(void);
extern void func_80050260(void);
extern int D_8009CF94;
extern int D_8009CF8C;
extern int D_8009CF98;
extern int D_8009CF00;
extern N *func_80062D2C(int a, int b, int c, int d);
extern N *func_8006322C(int a, N *b, N *c);
extern void func_80062CB8(N *n);
extern void func_80055760(void);
extern int func_80052F70(void);
extern void func_800647D0(N *n, int v);

void func_80044174(int arg0) {
    N *n1;
    N *n2;
    N *n3;
    int v;

    n1 = func_80062D2C(0x1B, 0, 0, 0);
    n1->f30 = func_800447F0;
    n2 = func_80062D2C(1, arg0, 0, 0);
    n3 = func_8006322C(1, n2, n2);
    n2->f2C = func_80044444;
    n3->f30 = func_8004F8D0;
    func_80062CB8(n3);
    n3->f84 = func_80057C54;
    n3->f88 = func_80050260;
    func_80055760();
    D_8009CF94 = -1;
    D_8009CF8C = -1;
    v = func_80052F70();
    func_800647D0(n3, v);
    v = D_8009CF98;
    D_8009CF00 = 0;
    if (v != 0) {
        /*
         * Retail computes the high byte with an in-place `sra $v1,$v1,8`
         * (result stays in $v1). cc1 otherwise reuses $v0 for the shift
         * result and only the destination register differs; pinning the
         * temp to $v1 reproduces retail exactly.
         */
        register int hi asm("$3");
        v--;
        n3->f44 = v & 1;
        n3->f48 = (v >> 1) & 0x7F;
        hi = v >> 8;
        n3->f5C = hi;
    }
}
