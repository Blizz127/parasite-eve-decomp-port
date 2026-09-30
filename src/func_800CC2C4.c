extern short D_800E2290[];
extern unsigned char *D_8009D254;
extern int func_80071A54();

void func_800CC2C4(int a0, int a1, unsigned char *a2) {
    int i;

    a2[3] = 0x7F;
    if (*(short *)(*(unsigned char **)(*(unsigned char **)D_8009D254 + 0x68) + 6) == 3) {
        *(short *)(a2 + 4) = 4;
    } else {
        *(short *)(a2 + 4) = 0x10;
    }
    i = 0;
    while (i < *(short *)(a2 + 4)) {
        int r;
        *(short *)(a2 + i * 2 + 0x8) = D_800E2290[0];
        *(short *)(a2 + i * 2 + 0x28) = D_800E2290[1];
        *(short *)(a2 + i * 2 + 0x48) = D_800E2290[2];
        *(short *)(a2 + i * 2 + 0x68) = (func_80071A54() % 11 - 5) << 8;
        r = func_80071A54();
        *(short *)(a2 + i * 2 + 0x88) = (-(r % 16 + 0x14)) << 8;
        *(short *)(a2 + i * 2 + 0xA8) = (func_80071A54() % 11 - 5) << 8;
        i++;
    }
}
