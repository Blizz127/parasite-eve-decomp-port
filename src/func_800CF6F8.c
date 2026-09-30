extern int D_8009CDDC;
extern unsigned int D_8009CDD8;
extern unsigned int D_800B0E58[];
extern int func_80077A64(int a0, int a1, int a2, int a3);
extern void func_80077C84(void *a0, int a1, int a2, int a3);

void func_800CF6F8(unsigned int *a0, unsigned int *a1, int a2) {
    unsigned int *p;
    unsigned int base;

    if (a2 != 0xFF) {
        base = D_8009CDD8;
        p = (unsigned int *)(D_800B0E58[D_8009CDDC] + base);
        D_8009CDD8 = base + 8;
        func_80077C84(p, 0, 1, func_80077A64(0, a2, 0, 0) & 0xFFFF);
        if (a1 != 0) {
            ((unsigned char *)a1)[7] |= 2;
            a1[0] = (a1[0] & 0xFF000000) | (a0[0] & 0xFFFFFF);
            a0[0] = (a0[0] & 0xFF000000) | ((unsigned int)a1 & 0xFFFFFF);
        }
        p[0] = (p[0] & 0xFF000000) | (a0[0] & 0xFFFFFF);
        a0[0] = (a0[0] & 0xFF000000) | ((unsigned int)p & 0xFFFFFF);
    } else {
        if (a1 != 0) {
            a1[0] = (a1[0] & 0xFF000000) | (a0[0] & 0xFFFFFF);
            a0[0] = (a0[0] & 0xFF000000) | ((unsigned int)a1 & 0xFFFFFF);
        }
    }
}
