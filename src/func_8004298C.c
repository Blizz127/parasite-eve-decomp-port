extern unsigned char D_800A0ED4[];
extern int D_800A186C;

extern void func_80062CD0(int);

void func_8004298C(int a0, int a1) {
    unsigned char *p;
    unsigned char v;

    p = &D_800A0ED4[a0 * 1048];
    v = p[1];
    if (v == 0 || v == 0xC) {
        p[1] = 1;
        p[0xB] = 2;
        *(short *)(p + 0x16) = 0xA;
        func_80062CD0(0);
        D_800A186C = a1;
    }
}
