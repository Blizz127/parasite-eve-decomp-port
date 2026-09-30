extern unsigned char *D_8009D2F0;
extern void func_8003E0D0(unsigned char *p);
int func_80019260(void) {
    func_8003E0D0(D_8009D2F0 + 0x1B4);
    *(unsigned int *)(D_8009D2F0 + 0x18C) = 0;
    return 1;
}
