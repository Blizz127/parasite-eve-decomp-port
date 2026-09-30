extern unsigned int D_800BCD50[];
extern unsigned int D_800BCD54;
extern unsigned int D_800BCD6C;
extern unsigned int D_800BCD70;
extern unsigned int D_800BCD74;

void func_8008F1B0(unsigned char *a0, unsigned int a1) {
    D_800BCD50[0] &= ~a1;
    D_800BCD6C &= ~a1;
    D_800BCD70 &= ~a1;
    D_800BCD74 &= ~a1;
    D_800BCD54 &= ~a1;
    *(unsigned int *)(a0 + 0x2C) = 0;
    *(unsigned int *)(a0 + 0x28) = 0;
}
