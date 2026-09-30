extern unsigned char D_800B8AC0[];
extern unsigned char *D_8009D2C8;
extern void func_8008A354(int a0, unsigned char *a1);

void func_8008C3E4(unsigned char *a0) {
    func_8008A354(*(int *)(a0 + 4), D_800B8AC0);
    if (*(volatile int *)(a0 + 4) != 0) {
        int arg = *(int *)(a0 + 4);
        D_8009D2C8 += 0x68;
        func_8008A354(arg, D_800B8AC0 + 0x1AA0);
        D_8009D2C8 -= 0x68;
    }
}
