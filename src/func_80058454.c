extern short *D_8009D048;
extern int D_8009D050;
extern unsigned char *D_8009D058;
extern int D_8009D064;
extern int D_8009D078;
extern unsigned char D_8009D05C[];
extern unsigned char D_800C0E48[];
extern unsigned char D_800BEEAC[];
extern unsigned char D_8009DE64[];
extern short D_800A1FD4[];
extern unsigned char *func_8005DB44(int);
extern int func_80052F70(void);
extern int func_8005833C(int);
extern void func_80055760(void);

void func_80058454(void)
{
    int unused[2];
    int i;
    short *s;
    int v;
    unsigned char *p;
    unsigned char *q;
    short *lp;
    int idx;
    int bound;
    register unsigned char *base asm("$2");
    register int m asm("$3");

    i = 0;
    D_8009D048 = (short *)D_800C0E48;
    D_8009D050 = func_80052F70();
    D_8009D058 = D_8009D05C;
    D_8009D064 = 2;
    if (D_8009D078 > 0) {
        s = D_800A1FD4;
        do {
            v = *s;
            if ((unsigned int)(v - 0x100) < 0x80) {
                m = v << 5;
                base = D_800BEEAC;
                __asm__ __volatile__("" : "=r"(base) : "0"(base));
                p = (unsigned char *)(m + (int)base);
                goto d1;
            }
            if ((unsigned int)(v - 1) < 0xFF) {
                p = func_8005DB44(v - 1);
                goto d1;
            }
            if ((unsigned int)(v - 0x200) < 9) {
                m = v << 5;
                base = D_8009DE64;
                __asm__ __volatile__("" : "=r"(base) : "0"(base));
                p = (unsigned char *)(m + (int)base);
                goto d1;
            }
            p = 0;
        d1:
            if (p[6] < 0x10) {
                goto ins;
            }
            if ((unsigned int)(v - 0x100) < 0x80) {
                m = v << 5;
                base = D_800BEEAC;
                __asm__ __volatile__("" : "=r"(base) : "0"(base));
                q = (unsigned char *)(m + (int)base);
                goto d2;
            }
            if ((unsigned int)(v - 1) < 0xFF) {
                q = func_8005DB44(v - 1);
                goto d2;
            }
            if ((unsigned int)(v - 0x200) < 9) {
                m = v << 5;
                base = D_8009DE64;
                __asm__ __volatile__("" : "=r"(base) : "0"(base));
                q = (unsigned char *)(m + (int)base);
                goto d2;
            }
            q = 0;
        d2:
            if (q[6] >= 0x13) {
                goto ins;
            }
            if (func_8005833C(i) == 0) {
                *s = 0;
            }
            goto next;
        ins:
            if (v < 0x200) {
                lp = D_8009D048;
                while (lp < D_8009D048 + D_8009D050 && *lp != 0) {
                    lp++;
                }
                if (lp < D_8009D048 + D_8009D050) {
                    idx = lp - D_8009D048;
                    goto e1;
                }
                idx = -1;
            e1:
                if (idx >= 0) {
                    D_8009D048[idx] = v;
                    *s = 0;
                }
            }
        next:
            bound = D_8009D078;
            __asm__ __volatile__("" : "=r"(bound) : "0"(bound));
            i++;
            s++;
        } while (i < bound);
    }
    func_80055760();
}
