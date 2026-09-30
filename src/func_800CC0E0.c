extern unsigned char D_800E2280;
extern unsigned char D_800E2281;
extern unsigned char D_800E2282;
extern short D_800E2290[];
extern unsigned char *D_8009D254;
extern int *func_800C2B10();
extern int func_80077CF4();
extern int func_80077DC4();

void func_800CC0E0(int a0, int a1, unsigned char *a2) {
    int i;
    int sh;

    D_800E2280 = *func_800C2B10(3);
    D_800E2281 = *func_800C2B10(4);
    D_800E2282 = *func_800C2B10(5);
    a2[1] = 0x7F;
    if (*(short *)(*(unsigned char **)(*(unsigned char **)D_8009D254 + 0x68) + 6) == 3) {
        a2[2] = 8;
    } else {
        a2[2] = 0x10;
    }
    i = 0;
    while (i < *(signed char *)(a2 + 2)) {
        int m;
        *(short *)(a2 + i * 8 + 0x26) = D_800E2290[0];
        m = -0x1400;
        *(short *)(a2 + i * 8 + 0x28) = D_800E2290[1];
        *(short *)(a2 + i * 8 + 0x2A) = D_800E2290[2];
        sh = i << 9;
        if (*(signed char *)(a2 + 2) == 0x10) {
            sh = i << 8;
        }
        *(short *)(a2 + i * 8 + 0xA6) = func_80077CF4(sh);
        *(short *)(a2 + i * 8 + 0xA8) = m;
        *(short *)(a2 + i * 8 + 0xAA) = func_80077DC4(sh);
        *(short *)(a2 + i * 2 + 6) = 0x258;
        i++;
    }
}
