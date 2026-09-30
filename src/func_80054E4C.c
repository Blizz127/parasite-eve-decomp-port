extern unsigned char D_800C0E0C;
extern unsigned char D_800C0E48[];
extern unsigned char D_8009D05C[];
extern short *D_8009D048;
extern int D_8009D050;
extern unsigned char *D_8009D058;
extern int D_8009D064;
extern int D_8009D06C;
extern int func_80051E58(void);
extern int func_80052F70(void);

int func_80054E4C(int need)
{
    short *q;
    int cap;
    int c2;
    int used;

    D_8009D048 = (short *)D_800C0E48;
    D_8009D050 = func_80052F70();
    D_8009D058 = D_8009D05C;
    D_8009D064 = 2;
    c2 = (D_800C0E0C + func_80051E58() < 0x33) ? (D_800C0E0C + func_80051E58()) : 0x32;
    used = 0;
    D_8009D048 = (short *)D_800C0E48;
    cap = c2;
    D_8009D050 = func_80052F70();
    D_8009D058 = D_8009D05C;
    D_8009D064 = 2;
    q = D_8009D048;
    while (q < D_8009D048 + D_8009D050) {
        used += (*q != 0);
        q++;
    }
    D_8009D06C = need;
    return (cap - used) >= need;
}
