extern unsigned char *D_800F34F4;
extern unsigned char *D_800E2248;

int func_800C2DA0(int a0) {
    unsigned int i;

    i = a0 & 0xFFFF;
    D_800F34F4[i * 6 + 1] = 0;
    D_800E2248[6] = D_800E2248[6] - 1;
    return -((*(unsigned int *)(D_800E2248 + 4) & 0xFFFF0000) == 0x1000000);
}
