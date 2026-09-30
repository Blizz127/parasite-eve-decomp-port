extern unsigned int *D_8009D2C8;
extern unsigned int D_800BCD70;
extern void func_80089B28(void);

void func_80090408(unsigned char *a0, unsigned int a1) {
    unsigned int *p;

    if (*(unsigned short *)(a0 + 0x54) == 0) {
        p = D_8009D2C8;
        p[14] &= ~a1;
        if (*(unsigned int *)(a0 + 0x38) & 0x800) {
            p[12] &= ~(1 << *(unsigned short *)(a0 + 0x5C));
        }
    } else {
        D_800BCD70 &= ~a1;
    }
    func_80089B28();
}
