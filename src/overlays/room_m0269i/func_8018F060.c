void func_8018F160(void);

int func_8018F060(unsigned char *a0)
{
    *(void **)(a0 + 0xC) = func_8018F160;
    a0[3] = 0xFF;
    a0[0x1A] = 0;
    a0[0x28] = 0;
    return 0;
}
