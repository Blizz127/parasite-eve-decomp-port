int func_80192808(unsigned char *a0)
{
    int *p;

    *(unsigned char *)(*(volatile int *)(a0 + 8) + 0x25A) = 0;
    *(unsigned char *)(*(volatile int *)(a0 + 8) + 0x25B) = 0;
    p = *(int **)(a0 + 0x10);
    a0[0] = 4;
    if (p != 0) {
        *p = 0;
    }
    return 0;
}
