int func_801925AC(unsigned char *a0, int a1, unsigned int a2, int a3, int a4, int a5)
{
    switch (a2) {
    case 0x19:
        if (a1 == 1) {
            *(int *)(a0 + 0x10) = a3;
            *(int *)a3 = a1;
        }
        break;
    case 0x100:
        *(short *)(a0 + 0x22) = (short)a3;
        *(short *)(a0 + 0x24) = (short)a4;
        break;
    case 0x101:
        *(short *)(a0 + 0x28) = (short)a3;
        *(short *)(a0 + 0x2A) = (short)a4;
        *(short *)(a0 + 0x2C) = (short)a5;
        break;
    }
    return 0;
}
