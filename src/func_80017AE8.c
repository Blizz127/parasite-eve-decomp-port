extern unsigned int *D_8009D2F0;
extern void func_8001A680(void *a0, int a1);

int func_80017AE8(unsigned short **a0) {
    func_8001A680(D_8009D2F0, *a0[0]);
    D_8009D2F0[0x26] &= 0xFFFFFEFF;
    return 1;
}
