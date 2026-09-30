extern short *D_8009D048;
extern int D_8009D050;
extern unsigned char *D_8009D058;
extern int D_8009D064;
extern int D_8009D0A4;
extern int D_8009D0A8;
extern short *D_8009D0AC;
extern int D_8009D0B0;
extern void *D_8009D0B4;
extern unsigned char *D_8009D0B8;
extern unsigned char *D_8009D0BC;
extern unsigned char D_8009D05C[];
extern unsigned char D_800C0E48[];
extern unsigned char D_800923F8[];
extern unsigned char D_80092428[];
extern unsigned char D_80092440[];
extern unsigned char D_80092458[];
extern unsigned char D_80092468[];
extern signed char D_800C0E20[];
extern signed char D_800C0E22[];
extern int func_80052F70(void);
extern void func_800723A4(short *, int, int, void *);
extern void func_8005B248(void);
extern void func_8005B3A4(void);
extern void func_80055760(void);
extern void func_800532B4();
extern void func_8005AFFC();

void func_8005B500(int a0, int a1)
{
    int unused[4];
    short *p;
    short *lst;
    register int i asm("$3");
    int n;
    int v1;
    int v2;
    unsigned char *sel;

    switch (a0) {
    case 0:
        D_8009D0B8 = D_80092428;
        D_8009D0A8 = a1;
        break;
    case 1:
        D_8009D0B8 = D_800923F8;
        D_8009D0A4 = a1;
        break;
    case 2:
        D_8009D0B8 = D_80092440;
        if (a1 != 0) {
            sel = D_80092458;
            __asm__ __volatile__("" : "=r"(sel) : "0"(sel));
        } else {
            sel = D_80092468;
            __asm__ __volatile__("" : "=r"(sel) : "0"(sel));
        }
        D_8009D0BC = sel;
        break;
    }
    D_8009D048 = (short *)D_800C0E48;
    D_8009D050 = func_80052F70();
    D_8009D058 = D_8009D05C;
    D_8009D064 = 2;
    lst = D_8009D048;
    v1 = lst[D_800C0E20[0]];
    if (D_800C0E22[0] >= 0) {
        v2 = lst[D_800C0E22[0]];
        goto g2;
    }
    v2 = -1;
g2:
    D_8009D0B4 = (void *)func_800532B4;
    D_8009D0AC = D_8009D048;
    D_8009D0B0 = D_8009D050;
    func_800723A4(D_8009D048, D_8009D050, 2, (void *)func_8005AFFC);
    func_8005B248();
    func_8005B3A4();
    n = D_8009D050;
    if (n > 0) {
        i = 0;
        p = D_8009D048;
    l1:
        if (*p != v1) {
            i++;
            p++;
            if (i < n) {
                goto l1;
            }
        }
        if (i < D_8009D050) {
            D_800C0E20[0] = i;
        }
    }
    if (v2 >= 0) {
        n = D_8009D050;
        if (n > 0) {
            i = 0;
            p = D_8009D048;
        l2:
            if (*p != v2) {
                i++;
                p++;
                if (i < n) {
                    goto l2;
                }
            }
            if (i < D_8009D050) {
                D_800C0E22[0] = i;
            }
        }
    }
    func_80055760();
}
