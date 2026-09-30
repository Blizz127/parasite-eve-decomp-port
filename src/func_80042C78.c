extern int D_8009CED8;
extern int D_8009CEDC;
extern int D_8009CEE0;
extern int D_8009CEE4;
extern int D_8009CEEC;

extern void func_80042CC4(int, int);

void func_80042C78(void) {
    D_8009CED8 = 0;
    D_8009CEE0 = 0;
    D_8009CEE4 = 0;
    D_8009CEDC = 0x20;
    func_80042CC4(0x90, 0xFF);
    D_8009CEEC = 0x48;
}
