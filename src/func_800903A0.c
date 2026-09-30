extern unsigned int *D_8009D2C8;
extern unsigned int D_800BCD70;
extern void func_80089B28(void);

void func_800903A0(unsigned char *a0, unsigned int a1) {
    if (*(unsigned short *)(a0 + 0x54) == 0) {
        D_8009D2C8[14] |= a1;
    } else {
        D_800BCD70 |= a1;
    }
    func_80089B28();
}
