extern unsigned char D_800C0E0C[];
extern unsigned char D_800C0E0A;
extern int D_800C0E00[];
extern int D_800C0E24[];
extern short D_800C0E08[];
extern short D_800C0E06[];
extern unsigned char *D_8009D048;
extern int D_8009D050;
extern unsigned char D_800C0E48[];
extern int func_80052F70(void);
extern int *func_8005DBF8(void);
extern unsigned short *func_8005DBAC(int);
extern void func_8005247C(void);

void func_8005D994(int a0)
{
    unsigned char *base;
    short *q;
    short *n;
    int i;
    int v;
    int w;

    D_800C0E0C[0] = 0x32;
    base = D_800C0E48;
    if (D_8009D048 == base) {
        D_8009D050 = func_80052F70();
    }
    D_800C0E0A = 0x62;
    D_800C0E00[0] = func_8005DBF8()[D_800C0E0A];
    v = *func_8005DBAC(0x62);
    D_800C0E24[0] = 0xFFFFF;
    D_800C0E08[0] = v;
    D_800C0E06[0] = v;
    n = (short *)(base - 0x20);
    for (i = 0; i < 7; i++) {
        q = n;
        n = q + 1;
        if (a0 == 0 || (unsigned int)(i - 1) >= 2) {
            w = 0x3E8;
        } else {
            w = 0;
        }
        *q = w;
    }
    func_8005247C();
}
