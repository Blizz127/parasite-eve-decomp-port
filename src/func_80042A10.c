extern unsigned char D_800A0ED4[];
extern unsigned char D_800A12ED;
extern unsigned char D_800A0ED5;
extern int D_800A1838;

extern void func_80072774(int);

void func_80042A10(void) {
    unsigned char *p;

    for (p = D_800A0ED4; p < D_800A0ED4 + 0x830; p += 0x418) {
        if (p[1] == 8 || p[1] == 0xA) {
            func_80072774(*(int *)(p + 0xC));
            *(int *)(p + 0xC) = -1;
            p[1] = 0xC;
        }
    }
    D_800A12ED = 0;
    D_800A0ED5 = 0;
    D_800A1838 = 0;
}
