extern unsigned short D_800E27F8;
extern unsigned short D_800E27FA;
extern unsigned short D_800E27FC;
extern int func_80071A54(void);

void func_800CD1FC(int a0, int a1, short *a2)
{
    int u;
    a2[4] = D_800E27F8 + func_80071A54() % 201 - 0x64;
    *(volatile short *)&a2[5] = D_800E27FA + func_80071A54() % 101 - 0x32;
    u = D_800E27FC;
    *(volatile short *)&a2[2] = 0x7F;
    *(volatile short *)&a2[3] = 0x544;
    a2[6] = u;
}
