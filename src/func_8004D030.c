extern void (*D_8009CFFC)();

extern void func_80062F1C();
extern void func_800525EC();

int func_8004D030(int a0, int a1) {
    if ((a1 & 0x10040) != 0) {
        func_80062F1C();
        if (D_8009CFFC != 0) {
            D_8009CFFC();
            D_8009CFFC = 0;
        }
        func_800525EC();
    }
    return 1;
}
