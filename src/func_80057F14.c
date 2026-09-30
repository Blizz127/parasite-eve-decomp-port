extern short D_800A1FD4[];
extern unsigned char D_800BEEAC[];
extern unsigned char D_8009DE64[];
extern unsigned char D_800C20A4[];
extern unsigned char *func_8005DB44(int);
extern unsigned char *func_8005DC9C(int);
extern void func_800534E4(unsigned char *, unsigned char *);

void func_80057F14(int a0)
{
    int v;
    unsigned char *p;
    unsigned char *q;

    v = D_800A1FD4[a0];
    if (v < 0x200) {
        if ((unsigned int)(v - 0x100) < 0x80) {
            p = (unsigned char *)((v << 5) + (int)D_800BEEAC);
        } else if ((unsigned int)(v - 1) < 0xFF) {
            p = func_8005DB44(v - 1);
        } else if ((unsigned int)(v - 0x200) < 9) {
            p = (unsigned char *)((v << 5) + (int)D_8009DE64);
        } else {
            p = 0;
        }
        if (p != 0) {
            func_800534E4(p, func_8005DC9C(p[4] - 1));
        }
    } else {
        p = (unsigned char *)((v << 5) + (int)D_8009DE64);
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
}
