extern short *D_8009D048;
extern int D_8009D050;
extern unsigned char *D_8009D058;
extern int D_8009D064;
extern int D_8009D028;
extern unsigned char D_8009D05C[];
extern unsigned char D_800C0E48[];
extern unsigned char D_800C0EAC[];
extern unsigned char D_800BEEAC[];
extern unsigned char D_800BEEB0[];
extern unsigned char D_8009DE64[];
extern signed char D_800C0E20[];
extern signed char D_800C0E22;
extern unsigned char *func_8005DB44(int);
extern int func_80052F70(void);
extern void func_80059A40(int *);
extern void func_80054E4C(int);
extern void func_80054CF8(void);
extern void func_800512AC(int, int);
extern void func_80053D2C(int);

void func_8005D020(void)
{
    int local[4];
    int i;
    int n;
    int n2;
    int v;
    register int w asm("$5");
    register int v2 asm("$3");
    register int m asm("$3");
    int idx;
    unsigned int u;
    register short *p asm("$4");
    short *q;
    short *skip;
    register short *tq asm("$2");
    unsigned char *res;
    int c61;
    register unsigned char *addr asm("$2");
    register int c7 asm("$18");
    int nn;

    D_8009D048 = (short *)D_800C0E48;
    n = func_80052F70();
    addr = D_8009D05C;
    D_8009D058 = addr;
    D_8009D050 = n;
    D_8009D064 = 2;
    i = 0;
    if (n > 0) {
        c61 = 0x61;
        n2 = n;
        p = D_8009D048;
    l1:
        u = *(unsigned short *)p;
        if ((unsigned int)(u - 0x100) < 0x80) {
            if (D_800BEEB0[(int)(short)u * 0x20] == c61) {
                goto found;
            }
        }
        i++;
        p++;
        if (i < n2) {
            goto l1;
        }
    found:
        if (i < D_8009D050) {
            if (D_8009D048 == (short *)D_800C0E48 && D_800C0E22 == i) {
                func_80059A40(local);
            }
            v = D_8009D048[i];
            D_8009D048[i] = 0;
            if (v >= 0x100) {
                D_800C0EAC[(v - 0x100) << 5] = 0;
            }
            if (D_8009D048 == (short *)D_800C0E48 && D_800C0E22 == i) {
                D_800C0E22 = -1;
                func_80054E4C(local[0]);
                func_80054CF8();
                func_800512AC(3, 0);
            }
        }
    }
    func_80053D2C(0x93);
    tq = D_8009D048;
    __asm__ __volatile__("" : "=r"(tq) : "0"(tq));
    c7 = 7;
    __asm__ __volatile__("" : "=r"(c7) : "0"(c7));
    q = tq;
    skip = q - 1;
    nn = D_8009D050;
    if (q < q + nn) {
        do {
            if (q != skip) {
                v2 = *q;
                w = v2;
                if ((unsigned int)(v2 - 0x100) < 0x80) {
                    res = (unsigned char *)((v2 << 5) + (int)D_800BEEAC);
                    goto d1;
                }
                if ((unsigned int)(v2 - 1) < 0xFF) {
                    res = func_8005DB44(v2 - 1);
                    goto d1;
                }
                if ((unsigned int)(w - 0x200) < 9) {
                    m = w << 5;
                    res = (unsigned char *)(m + (int)D_8009DE64);
                    goto d1;
                }
                res = 0;
            d1:
                if (res[6] == c7) {
                    break;
                }
            }
            q++;
        } while (q < D_8009D048 + D_8009D050);
        if (q < D_8009D048 + D_8009D050) {
            idx = q - D_8009D048;
            goto d2;
        }
    }
    idx = -1;
d2:
    D_800C0E20[0] = idx;
    D_8009D028 = 0;
    func_800512AC(2, 0);
}
