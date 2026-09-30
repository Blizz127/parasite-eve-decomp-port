void func_80192654(void);

int func_80192584(unsigned char *a0)
{
    *(void **)(a0 + 0xC) = func_80192654;
    a0[0x1A] = 0;
    *(int *)(a0 + 0x10) = 0;
    *(short *)(a0 + 0x1C) = 0;
    *(short *)(a0 + 0x20) = 0;
    *(short *)(a0 + 0x26) = 0;
    return 0;
}
