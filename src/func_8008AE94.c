extern unsigned short D_800B89BC;
extern unsigned char *D_8009D2C8;
extern void func_8008AC40(int a0);
extern void func_8008A068(int a0);

void func_8008AE94(unsigned char *a0)
{
    unsigned short t;

    t = D_800B89BC;
    if (t != 0 && t == *(int *)(a0 + 0xC)) {
        func_8008AC40(*(int *)(a0 + 4));
    } else {
        func_8008A068(*(int *)(a0 + 4));
        *(short *)(D_8009D2C8 + 0x54) = *(int *)(a0 + 0xC);
    }
}
