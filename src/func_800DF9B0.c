extern int D_800E27EC;
extern int D_800F32D8;
extern unsigned char *D_800F33E0;
extern void *D_8009D254;
extern unsigned char D_800E220C[];
extern int func_80071A54();
extern void func_800C6D5C();
extern int func_800CE560();
extern unsigned char *func_800CE610();
extern void func_800CE870();
extern void func_800D9A8C();

int func_800DF9B0(int a0, int *a1) {
    switch (a0) {
    case 0:
        {
            int r = func_80071A54();
            int d = D_800F32D8;
            *a1 = r;
            func_800C6D5C(d, 0, 0);
        }
        return func_800CE560(*(void **)(D_800F33E0 + 8), 0x10, 0x18, func_800D9A8C);
    case 1: {
        int x = D_800E27EC;
        if (x < 0x28 && x % 6 == 0) {
            unsigned char *t = func_800CE610(*(void **)(D_800F33E0 + 8));
            if (t != 0) {
                int v = *a1;
                int r;
                *(short *)(t + 6) = 0;
                *(short *)(t + 0xC) = 0;
                *(short *)(t + 0xA) = v;
                r = func_80071A54();
                {
                    int w = *a1 - 0x555;
                    *a1 = w - (r & 0xFF);
                }
            }
        }
        if (D_800E27EC >= 0x46) {
            return 1;
        }
        break;
    }
    case 2:
        func_800CE870(D_8009D254, 1, &D_800E220C);
        break;
    }
    return 0;
}
