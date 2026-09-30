extern short *D_8009D048;
extern int D_8009D050;
extern int *D_8009D058;
extern int D_8009D064;
extern int D_8009D044;
extern unsigned char D_800C0E48[];
extern int D_8009D05C[];
extern short D_800A1E00[];
extern unsigned char D_800BEEAC[];
extern unsigned char D_8009DE64[];
extern unsigned char *func_8005DB44(int);
extern int func_80052F70(void);
extern void func_80055760(void);

void func_80058C4C(int mask)
{
    int unused[4];
    short *out;
    int i;
    int n;
    int v;
    int w;
    unsigned char *p;
    register int sh asm("$2");
    int bit;

    out = D_800A1E00;
    D_8009D048 = (short *)D_800C0E48;
    n = func_80052F70();
    D_8009D058 = D_8009D05C;
    D_8009D050 = n;
    D_8009D064 = 2;
    i = 0;
    if (n > 0) {
        do {
            if (i >= 0 && i < D_8009D050) {
                v = D_8009D048[i];
                w = v;
                if ((unsigned int)(v - 0x100) < 0x80) {
                    p = (unsigned char *)((v << 5) + (int)D_800BEEAC);
                    goto got;
                }
                if ((unsigned int)(v - 1) < 0xFF) {
                    p = func_8005DB44(v - 1);
                    goto got;
                }
                if ((unsigned int)(w - 0x200) < 9) {
                    p = (unsigned char *)((w << 5) + (int)D_8009DE64);
                    goto got;
                }
            }
            p = 0;
        got:
            if (p == 0) {
                goto zsh;
            }
            sh = p[6];
            bit = mask >> sh;
            goto have;
        zsh:
            sh = 0;
            asm volatile("" : "=r"(sh) : "0"(sh));
            bit = mask >> sh;
        have:
            if (bit & 1) {
                *out = i;
                out++;
            }
            i++;
        } while (i < D_8009D050);
    }
    D_8009D044 = out - D_800A1E00;
    func_80055760();
    i = 0;
    if (D_8009D050 > 0) {
        do {
            if (D_8009D048[i] == 0) {
                *out = i;
                out++;
            }
            i++;
        } while (i < D_8009D050);
    }
    D_8009D044 = out - D_800A1E00;
}
