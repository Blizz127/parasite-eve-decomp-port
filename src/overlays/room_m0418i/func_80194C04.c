extern int *func_800C2B50();

void func_80194C04(int a0, int a1, int *s)
{
    int *r;
    int w;

    r = func_800C2B50();
    s[0] = r[6] - 0x15E;
    s[1] = r[7];
    w = r[8];
    *(short *)((char *)s + 0x10) = 0;
    *(short *)((char *)s + 0x12) = 0;
    *(short *)((char *)s + 0x14) = 0xFF;
    s[2] = w;
}
