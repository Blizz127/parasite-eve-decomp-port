extern unsigned char D_800A0ED4[];
extern int D_800A1860;

extern void func_8004DAA4();

int func_80042848(int a0) {
    int f;

    f = D_800A0ED4[a0 * 1048] & 4;
    if (f != 0) {
        if (D_800A1860 == 0) {
            D_800A1860 = a0 + 1;
            func_8004DAA4();
        }
    }
    return f == 0;
}
