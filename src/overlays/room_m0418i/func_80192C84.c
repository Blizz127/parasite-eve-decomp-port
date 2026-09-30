extern unsigned char D_8018F04C[];
void func_800D1AE0(unsigned char *a0, int a1, int a2, int a3);

void func_80192C84(int a0, int a1, short *a2)
{
    unsigned char buf[4];

    __builtin_memcpy(buf, D_8018F04C, 4);
    func_800D1AE0(buf, a2[1], 1, 8);
}
