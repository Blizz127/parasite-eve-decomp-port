extern unsigned char D_8009CD78;
extern unsigned char D_8009CD80;
extern unsigned char D_800917E4;
extern unsigned char D_8009180C;
extern unsigned char D_8009173C;
extern volatile int D_8009D280;
extern volatile int D_8009D1C4;
extern unsigned int D_8009D1A0;
extern unsigned int D_800B0CD8;

extern int func_80038D0C();
extern int func_80039678();
extern int func_80038D74();
extern int func_800392EC();
extern void func_80039970();
extern int func_8006E3D4();
extern int func_80070DD0();

int func_80015790(unsigned char **a0) {
    int v;
    int k;
    register unsigned char *arg asm("$4");

    if ((func_80038D0C() & 0xFF) != 0) {
        v = func_80039678(**a0);
    } else {
        v = func_80038D74();
    }
    k = v & 0xFF;
    if (k == 0xFF) {
        if (((unsigned int)func_800392EC() & 0xFF) < 2) {
            func_80039970();
            arg = &D_8009CD78;
        } else {
            func_80039970();
            arg = &D_8009CD80;
        }
        D_8009D280 = func_8006E3D4(arg);
        return 1;
    }
    if (((unsigned int)(v - 6) & 0xFF) < 2 || k == 8) {
        if ((short)func_80070DD0(0, 0x64) < 0x3C) {
            arg = &D_800917E4;
        } else {
            arg = &D_800917E4 + ((int)(short)func_80070DD0(1, 4) << 3);
        }
    } else {
        register unsigned int t asm("$3");
        register unsigned int q asm("$2");
        unsigned int m;
        t = ((unsigned int)func_800392EC() - 1) & 0xFF;
        q = t / 10;
        t = q & 0xFF;
        __asm__ __volatile__("" : "=r"(t) : "0"(t));
        m = t * 24;
        {
            unsigned char *tb = &D_8009173C;
            arg = (unsigned char *)((*(tb + m + k) << 3) + (unsigned int)&D_8009180C);
        }
    }
    D_8009D280 = func_8006E3D4(arg);
    {
    int r = 1;
    if (D_8009D1C4 == D_8009D280) {
        register unsigned int *p asm("$3");
        register unsigned int x asm("$4");
        register unsigned int y asm("$5");
        p = &D_800B0CD8;
        x = D_8009D1A0;
        y = *p;
        D_8009D1A0 = x | 0x2000;
        *p = y | 0x800;
    }
    return r;
    }
}
