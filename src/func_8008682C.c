extern int D_800BCD80;
extern void func_8008CBA8(void);
void func_8008682C(int a0) {
    int v;
    switch (a0) {
    case 1: v = 0x9A; break;
    case 2: v = 0x9C; break;
    default: v = 0x98; break;
    }
    D_800BCD80 = v;
    func_8008CBA8();
}
