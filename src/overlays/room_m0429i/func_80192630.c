int func_80192630(unsigned char *a0, int a1, int a2, int *a3)
{
    if (a2 == 0x19) {
        if (a1 == 1) {
            *(int **)(a0 + 0x10) = a3;
            *a3 = a1;
        }
    }
    return 0;
}
