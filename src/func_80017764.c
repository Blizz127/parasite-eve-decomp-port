extern unsigned int *D_8009D2F0;
extern void func_800653B8(int a0, int a1, int a2, int a3, int a4);

int func_80017764(unsigned char **a0) {
    func_800653B8(*a0[2], *a0[1], *(unsigned short *)a0[0],
                  ((unsigned short *)D_8009D2F0)[0x12], 0);
    return 1;
}
