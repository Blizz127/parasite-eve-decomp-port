extern void func_8009021C(unsigned char *a0, unsigned int a1);
extern void func_80090328(unsigned char *a0, unsigned int a1);
extern void func_80090408(unsigned char *a0, unsigned int a1);

void func_80090BCC(unsigned char *a0, unsigned int a1) {
    *(unsigned int *)(a0 + 0x38) &= ~0x37;
    func_8009021C(a0, a1);
    func_80090328(a0, a1);
    func_80090408(a0, a1);
    *(unsigned short *)(a0 + 0x84) &= 0xFFFA;
}
