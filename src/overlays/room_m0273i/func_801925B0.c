extern unsigned char D_8019A8E0[];

int func_801925B0(unsigned char *a0, int a1, int a2, int a3, int a4)
{
    switch (a2) {
    case 0x19:
        if (a1 == 1) {
            *(int *)(a0 + 0x10) = a3;
            *(int *)a3 = a1;
        }
        break;
    case 0x1D:
        *(short *)(a0 + 0x14) = (short)a3;
        a0[0x4C] = D_8019A8E0[a4];
        break;
    }
    return 0;
}
