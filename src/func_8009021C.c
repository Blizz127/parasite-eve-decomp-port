extern unsigned int *D_8009D2C8;
extern unsigned int D_800BCD6C;
extern unsigned int D_8009D2C4;
extern void func_80089960(void);

void func_8009021C(unsigned char *a0, unsigned int a1) {
    if (*(unsigned short *)(a0 + 0x54) == 0) {
        D_8009D2C8[13] &= ~a1;
    } else {
        D_800BCD6C &= ~a1;
    }
    D_8009D2C4 |= 0x10;
    func_80089960();
    *(unsigned short *)(a0 + 0xBA) = 0;
}
