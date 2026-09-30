extern unsigned char D_800A0ED4[];

extern void func_80072774(int);

void func_80042798(void) {
    unsigned char *p;

    for (p = D_800A0ED4; p < D_800A0ED4 + 0x830; p += 0x418) {
        if (p[1] == 8 || p[1] == 0xA) {
            func_80072774(*(int *)(p + 0xC));
            *(int *)(p + 0xC) = -1;
            p[1] = 0xC;
        }
    }
}
