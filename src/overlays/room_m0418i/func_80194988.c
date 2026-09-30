extern int *func_800C2B50();

void func_80194988(int a0, int a1, int *s)
{
    int *r;
    int w;

    r = func_800C2B50();
    s[0] = r[6] - 0xC8;
    s[1] = r[7];
    w = r[8];
    *(short *)((char *)s + 0x10) = 0;
    *(short *)((char *)s + 0x12) = 0;
    *(short *)((char *)s + 0x14) = 0;
    *(short *)((char *)s + 0x1C) = 0xC04;
    *((char *)s + 0x22) = 0;
    *(short *)((char *)s + 0x1E) = 0;
    *(short *)((char *)s + 0x20) = 0;
    s[2] = w;
}
