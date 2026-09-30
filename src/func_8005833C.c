extern int D_8009D078;
extern short D_800A1FD4[];
extern unsigned char D_800BEEAC[];
extern unsigned char D_8009DE64[];
extern unsigned char *D_8009D048;
extern int D_8009D050;
extern int *D_8009D058;
extern int D_8009D064;
extern unsigned char D_800C0E48[];
extern int D_8009D05C[];
extern unsigned char *func_8005DB44(int);
extern int func_80052F70(void);
extern int func_80053B48(unsigned char *);

int func_8005833C(int i)
{
    int v;
    int w;
    unsigned char *p;
    int r;

    if (i < 0 || i >= D_8009D078) {
        goto notok;
    }
    v = D_800A1FD4[i];
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
    p = 0;
got:
    D_8009D048 = D_800C0E48;
    D_8009D050 = func_80052F70();
    D_8009D058 = D_8009D05C;
    D_8009D064 = 2;
    r = func_80053B48(p);
    if (r == 0) {
        D_800A1FD4[i] = 0;
    }
    goto done;
notok:
    r = 1;
done:
    return r;
}
