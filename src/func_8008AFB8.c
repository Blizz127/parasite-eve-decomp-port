extern unsigned char *D_8009D2C8;
extern unsigned char D_800B8AC0[];
extern void func_8008D820(unsigned char *a0, unsigned char *a1, int a2);
extern void func_8008A068(int a0);

void func_8008AFB8(unsigned char *a0)
{
    unsigned char *p;

    p = D_8009D2C8;
    if (*(int *)(p + 4) != 0 && *(int *)(p + 0x6C) == 0) {
        func_8008D820(p, p + 0x68, 0x68);
        func_8008D820(D_800B8AC0, D_800B8AC0 + 0x1AA0, 0x1AA0);
    }
    func_8008A068(*(int *)(a0 + 4));
    *(short *)(D_8009D2C8 + 0x54) = *(int *)(a0 + 0xC);
}
