extern unsigned char D_800B0DC5;
extern int D_800B0CD8;
extern unsigned char D_800B0CE2;
extern unsigned char D_800B0CE3;
extern signed char D_800B0CE4;
extern unsigned char D_800B0CE6;
extern unsigned char D_800B0CEB;
extern int *D_8009D254;
extern int D_800B0E70;
extern int D_800B0EEC;
extern unsigned char D_800BEA40[];
extern unsigned char D_800B89F8[];
extern int func_8006BECC(void);
extern void func_8006C4C4(int a0);
extern int func_8006C5BC(void);
extern void func_8001A680(int *a0, int a1);
extern void func_8003D050(int *a0, int a1, int a2, int a3, int a4, int a5,
                          int a6, int a7, int *a8, int a9);
extern void func_8006698C(int *a0);
extern void func_8003D834(int *a0, int a1, int a2, unsigned char *a3,
                          unsigned char *a4);

int func_8006C1CC(int a0)
{
    int buf[2];
    register unsigned char *b asm("$16");
    register int p1 asm("$17");
    int *q;
    register int *r1 asm("$4");
    int *r2;
    int *r3;
    int *r4;
    int *r5;
    unsigned char *s;
    register unsigned int u asm("$5");
    register int c3 asm("$4");
    register int w asm("$3");
    int st;
    int rc;
    register int sh1 asm("$2");
    register int t3 asm("$3");
    register int tv asm("$2");

    p1 = a0;
    b = (unsigned char *)&D_800B0CD8;
    switch (D_800B0DC5) {
    case 0x20:
        q = &D_800B0CD8;
        *q = *q | 0x20000;
        if (p1 != 0) {
            w = D_800B0CE2;
            D_800B0CE2 = 0xE;
            D_800B0CEB = w;
        } else {
            D_800B0CE2 = D_800B0CEB;
        }
        st = 0x21;
        goto store;
    case 0x21:
        st = 0x22;
        goto store;
    case 0x22:
        st = 0x23;
        goto store;
    case 0x23:
        w = D_800B0CE2;
        u = w - 0xA;
        if (u < 5) {
            c3 = D_800B0CE3;
            if (w != c3) {
                D_800B0CD8 = D_800B0CD8 | 0x200000;
            }
            sh1 = u >> 1;
            t3 = c3 - 0xA;
            t3 = t3 / 2;
            if (sh1 != t3) {
                D_800B0CE6 = D_800B0CE6 | 4;
            }
        }
        st = 0x24;
        goto store;
    case 0x24:
        rc = func_8006BECC();
        if (rc == 1) {
            goto done1;
        }
        if (p1 != 0) {
            goto set27;
        }
        st = 0x25;
        goto store;
    case 0x25:
        s = &D_800B0CE6;
        w = *s;
        *s = w | 4;
        func_8006C4C4(D_800B0CE4);
        b[0xED] = 0x26;
        return 1;
    case 0x26:
        rc = func_8006C5BC();
        if (rc != 1) {
            st = 0x27;
            goto store;
        }
        if (b[0xEE] < 0xB) {
            goto done1;
        }
    set27:
        st = 0x27;
    store:
        b[0xED] = st;
    done1:
        return 1;
    case 0x27:
        r1 = D_8009D254;
        tv = D_800B0E70;
        r1[0x1AC / 4] = tv;
        tv = D_800B0EEC;
        r1[0x1B0 / 4] = tv;
        func_8001A680(r1, 0x15);
        r2 = D_8009D254;
        func_8003D050((int *)((unsigned char *)r2 + 0x1B4), r2[0x1AC / 4],
                      r2[0x278 / 4] + 0x50, 0x3C0, 0x100, 0, 0x1C0, 2, buf, 1);
        r3 = D_8009D254;
        func_8006698C((int *)((unsigned char *)r3 + 0x1B4));
        r4 = D_8009D254;
        func_8003D834((int *)((unsigned char *)r4 + 0x1B4), r4[0x1B0 / 4], 0,
                      D_800BEA40, D_800B89F8);
        r5 = D_8009D254;
        *(short *)(*(int *)((unsigned char *)r5 + 0x1B4) + 0x14) =
            *(short *)((unsigned char *)r5 + 0x224) * 2;
        D_800B0CD8 = D_800B0CD8 & 0xFFF9FFFF;
        if (p1 != 0) {
            *(int *)b = *(int *)b | 0x80000;
        } else {
            *(int *)b = *(int *)b & 0xFFF7FFFF;
        }
        b[0xED] = 0x20;
        return 0;
    }
    return 0;
}
