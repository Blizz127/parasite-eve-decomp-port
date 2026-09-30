extern unsigned int D_800B0CD8;
extern unsigned char D_800B0CE6;
extern void func_80074F44(short *a0, int a1, int a2, int a3);
extern void func_80074DC0(int a0);
extern void func_80068B94(void);

void func_8006E60C(void)
{
    short buf[4];
    unsigned int *f = &D_800B0CD8;

    if ((*f & 0x8000000) != 0) {
        buf[2] = 0x140;
        buf[0] = 0;
        buf[1] = 0;
        buf[3] = 0x1C0;
        func_80074F44(buf, 0, 0, 1);
        func_80074DC0(0);
        func_80068B94();
        D_800B0CE6 = D_800B0CE6 | 2;
        *f = *f & 0xF7FFFDFF;
    }
}
