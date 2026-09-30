extern short *D_8009D048;
extern int D_8009D050;
extern int D_8009D06C;
extern unsigned char D_800C0E0C;
extern signed char D_800C0E20[];
extern int func_80051E58(void);

void func_80054CF8(void)
{
    signed char *s;
    short *p;
    int t;
    int slot;
    int cap;
    int k;

    for (; D_8009D06C > 0; D_8009D06C--) {
        s = D_800C0E20;
        p = D_8009D048;
        while (p < D_8009D048 + D_8009D050) {
            if (*p == 0) {
                break;
            }
            p++;
        }
        if (p < D_8009D048 + D_8009D050) {
            t = p - D_8009D048;
        } else {
            t = -1;
        }
        slot = t;
        if (slot >= 0) {
            cap = (D_800C0E0C + func_80051E58() < 0x33) ? (D_800C0E0C + func_80051E58()) : 0x32;
            k = cap - D_8009D06C;
            D_8009D048[slot] = D_8009D048[k];
            D_8009D048[k] = 0;
            if (s[0] == k) {
                s[0] = slot;
            } else if (s[2] == k) {
                s[2] = slot;
            }
        }
    }
}
