extern short D_800E2850;
extern short D_800E2852;
extern int func_80077A64(int a0, int a1, int a2, int a3);

void func_800CECAC(void)
{
    int i;
    short *p;

    for (i = 0, p = &D_800E2850; i < 2; i++, p += 2) {
        *p = func_80077A64(i, 0, 0x380, 0x100);
    }
    for (i = 0, p = &D_800E2852; i < 2; i++, p += 2) {
        *p = func_80077A64(i, 0, 0x340, 0x100);
    }
}
