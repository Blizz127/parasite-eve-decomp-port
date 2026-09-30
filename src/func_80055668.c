extern int D_800C0E24[];
extern unsigned short *func_8005DC10(void);
extern void func_8004C4B4(int);

int func_80055668(int a0)
{
    unsigned short *p;
    int i;
    int r;

    p = func_8005DC10();
    for (i = 0; i < 20; i++) {
        if (p[i * 2] == a0) {
            break;
        }
    }
    r = i < 20;
    if (r) {
        D_800C0E24[0] |= 1 << i;
        func_8004C4B4(i);
    }
    return r;
}
