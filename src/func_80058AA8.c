extern short *D_8009D07C;
extern unsigned int *D_8009D058;
extern unsigned char D_800BEEAC[];
extern unsigned char D_8009DE64[];
extern unsigned char D_800C20A4[];
extern unsigned char *func_8005DB44(int);
extern unsigned char *func_8005DC9C(int);
extern void func_8005EB58(int);
extern void func_800534E4(unsigned char *, unsigned char *);

void func_80058AA8(int a0)
{
    int v;
    unsigned char *p;
    unsigned char *q;
    unsigned char *res;

    v = D_8009D07C[a0];
    if (v == 0) {
        return;
    }
    func_8005EB58((D_8009D058[a0 >> 5] & (1 << (a0 & 0x1F))) == 0);
    if ((unsigned int)(v - 0x100) < 0x80) {
        res = (unsigned char *)((v << 5) + (int)D_800BEEAC);
        goto done;
    }
    if ((unsigned int)(v - 1) < 0xFF) {
        res = func_8005DB44(v - 1);
        goto done;
    }
    if ((unsigned int)(v - 0x200) < 9) {
        res = (unsigned char *)((v << 5) + (int)D_8009DE64);
        goto done;
    }
    res = 0;
done:
    p = res;
    if (p[5] & 0x10) {
        q = D_800C20A4;
        if (p[6] == 9) {
            q += 0x10;
        }
    } else {
        q = func_8005DC9C(p[4] - 1);
    }
    func_800534E4(p, q);
}
