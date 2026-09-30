extern unsigned char D_800C0E00[];
extern unsigned char D_800C0E0A;
extern unsigned short D_800C0E06;
extern int D_800C0E10;
extern int D_800A18D8[];
extern int D_800A18B4[];
extern int D_800A18FC[];
extern int D_8009CFE8;
extern int D_8009CFEC;
extern int D_8009CF60;
extern int D_8009CFF0;
extern int D_8009CF6C;
extern int D_8009CF64;
extern int D_8009CF70;
extern int D_8009CF68;
extern int D_8009CF74;
extern int D_8009CF84;

extern void func_80051510();
extern void func_8005B91C(int, int, int *, int);
extern void func_80057E14(int);
extern void func_8004BE4C();
extern unsigned char *func_80062A34(int, int);
extern void func_80063158(unsigned char *, int, int);
extern void func_8005270C();
extern void func_8005C144();

void func_8004BCE8(int a0) {
    int loc;
    short *src;
    int i;
    int *pz;
    int *pb;
    int *pd;
    int base;
    int cnt;
    int h;
    int w;
    int v;

    func_80051510();
    base = *(int *)D_800C0E00;
    cnt = D_800C0E0A;
    h = D_800C0E06;
    w = D_800C0E10;
    D_8009CFE8 = base;
    D_8009CFEC = base;
    D_8009CF60 = cnt;
    D_8009CFF0 = cnt;
    D_8009CF6C = cnt;
    D_8009CF64 = h;
    D_8009CF70 = h;
    D_8009CF68 = w;
    if (a0 < 0) {
        a0 = 0;
    }
    D_8009CF74 = w + a0;
    src = (short *)(D_800C0E00 + 0x28);
    i = 0;
    pz = D_800A18FC;
    pb = D_800A18B4;
    pd = D_800A18D8;
    do {
        v = *src;
        src++;
        *pd = v;
        func_8005B91C(i, v, pb, 0);
        pb++;
        i++;
        *pz = 0;
        pz++;
        pd++;
    } while (i < 7);
    func_8005B91C(0, D_800A18D8[0], &loc, 0);
    D_8009CF84 = 2;
    func_80057E14(0);
    func_8004BE4C();
    func_80063158(func_80062A34(1, 0x15), 0, 0x20);
    if (a0 > 0) {
        func_8005270C();
    }
    func_8005C144();
}
