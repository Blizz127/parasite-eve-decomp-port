extern unsigned short D_800E21C8;
extern int func_80071A54(void);

void func_800CFFAC(short *a0)
{
    int v;
    int x;
    int y;

    D_800E21C8 = (D_800E21C8 + 1) & 7;
    v = func_80071A54();
    v = (v & 0x1FF) + ((D_800E21C8 << 9) & 0xC00);
    x = v + 0x100;
    if (D_800E21C8 & 1) {
        y = (func_80071A54() & 0x1FF) + 0x100;
    } else {
        y = -(func_80071A54() & 0x1FF) - 0x100;
    }
    a0[0] = x;
    a0[1] = y;
    a0[2] = 0;
}
