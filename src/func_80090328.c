extern unsigned int *D_8009D2C8;
extern unsigned int D_800BCD74;
extern void func_80089CF0(void);

void func_80090328(unsigned char *a0, unsigned int a1) {
    if (*(unsigned short *)(a0 + 0x54) == 0) {
        D_8009D2C8[15] &= ~a1;
    } else {
        D_800BCD74 &= ~a1;
    }
    func_80089CF0();
    *(unsigned short *)(a0 + 0xBC) = 0;
}
