extern unsigned char D_800C0EAC[];
extern unsigned char D_800C0E48[];
extern short *D_8009D048;
extern int D_8009D050;
extern unsigned char *D_8009D058;
extern unsigned char D_8009D05C[];
extern int D_8009D064;
extern unsigned char *func_8005DB44(int);
extern int func_80052F70(void);

typedef struct { unsigned char b[32]; } Rec;

unsigned char *func_80053968(int a0)
{
    unsigned char *p;
    unsigned char *res;
    short *q;
    int i;
    int j;
    int t;

    res = 0;
    p = D_800C0EAC;
    while (p < D_800C0EAC + 0x1000 && *p != 0) {
        p += 0x20;
    }
    if (p < D_800C0EAC + 0x1000) {
        t = (p - D_800C0EAC) >> 5;
        goto d1;
    }
    t = -1;
d1:
    i = t;
    q = D_8009D048;
    while (q < D_8009D048 + D_8009D050 && *q != 0) {
        q++;
    }
    if (q < D_8009D048 + D_8009D050) {
        j = q - D_8009D048;
        goto d2;
    }
    j = -1;
d2:
    if (i >= 0 && j >= 0) {
        unsigned char *base = D_800C0EAC;
        res = base + (i << 5);
        *(Rec *)res = *(Rec *)func_8005DB44(a0 - 1);
        D_8009D048 = (short *)(base - 0x64);
        D_8009D050 = func_80052F70();
        D_8009D058 = D_8009D05C;
        D_8009D064 = 2;
        D_8009D048[j] = i + 0x100;
    }
    return res;
}
