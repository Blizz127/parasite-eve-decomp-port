extern unsigned char D_800A1878[];
extern int D_8009CEE0;

void func_80042CC4(int a0, int a1) {
    unsigned char *p;
    int step;
    int base;

    D_800A1878[0] = 0;
    step = 0x100 - a0;
    base = a0 << 8;
    for (p = D_800A1878; p < D_800A1878 + 0xF; p++) {
        if (*p >= a1) {
            break;
        }
        p[1] = (base + step * *p) >> 8;
    }
    D_8009CEE0 = (p - D_800A1878) + 1;
}
