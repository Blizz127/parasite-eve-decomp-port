extern unsigned char D_800C0DE0[];
extern unsigned char D_800C0DF0[];
extern unsigned char *D_8009D0C0;
extern int D_8009D0C4;
extern unsigned char *D_8009D0C8;
extern int D_8009D218;
extern short D_800C1F80[];
extern int D_800C0E44[];
extern int D_800A76A4[];
extern int D_800A76B0[];
extern int D_800A76BC[];
extern int D_800A76C8[];
extern unsigned char D_800C20A4[];
extern unsigned char D_800C20B4[];
extern void func_80071A24(unsigned char *, int);
extern unsigned char *func_8005DC9C(int);
extern unsigned char *func_8005DC4C(int);
extern void func_80052594(unsigned char *);
extern void func_8005CCA4(void);
extern void func_800614AC(int);
extern int func_8005E884(void);
extern void func_8005E850(int, int);
extern void func_800649D0(int);
extern void func_80052790(int);

void func_8005D6F4(void)
{
    unsigned char *q;
    unsigned char *src;
    unsigned char *dst;
    unsigned char *d0;
    unsigned char *r;
    int c;
    register int fe asm("$2");
    int fe2;
    register unsigned char *t0 asm("$2");
    register unsigned char *t1 asm("$2");

    func_80071A24(D_800C0DE0, 0x12E4);
    t0 = D_800C0DE0 + 0x10;
    q = t0;
    D_8009D218 = 1;
    D_8009D0C8 = 0;
    D_8009D0C0 = q;
    D_8009D0C4 = 8;
    if (q < D_8009D0C0 + D_8009D0C4) {
        do {
            *q = 0xFF;
            q++;
        } while (q < D_8009D0C0 + D_8009D0C4);
    }
    d0 = D_8009D0C0;
    if (D_8009D0C8 != 0) {
        r = func_8005DC9C(D_8009D0C8[4] - 1);
    } else {
        r = func_8005DC4C(0x1E);
    }
    dst = d0;
    src = r;
    while ((*dst++ = *src++) != 0xFF) {
        ;
    }
    t1 = D_800C0DF0;
    q = t1;
    D_8009D218 = 1;
    D_8009D0C8 = 0;
    D_8009D0C0 = q;
    D_8009D0C4 = 8;
    if (q < D_8009D0C0 + D_8009D0C4) {
        do {
            *q = 0xFF;
            q++;
        } while (q < D_8009D0C0 + D_8009D0C4);
    }
    d0 = D_8009D0C0;
    if (D_8009D0C8 != 0) {
        r = func_8005DC9C(D_8009D0C8[4] - 1);
    } else {
        r = func_8005DC4C(0x1E);
    }
    dst = d0;
    src = r;
    while ((*dst++ = *src++) != 0xFF) {
        ;
    }
    func_80052594(func_8005DC4C(0x1E));
    func_8005CCA4();
    D_800C1F80[0] = 0x203;
    D_800C0E44[0] = 0x404040;
    func_800614AC(0x404040);
    D_800A76A4[0] = 0;
    D_800A76B0[0] = 0;
    D_800A76BC[0] = 0;
    D_800A76C8[0] = 0;
    func_8005E850(0, 8 - func_8005E884());
    func_800649D0(0);
    func_80052790(1);
    D_800C20A4[0] = 0xFF;
    D_800C20B4[0] = 0xFF;
}
