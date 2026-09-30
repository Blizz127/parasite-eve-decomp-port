extern unsigned short D_800E2290;
extern unsigned short D_800E2292;
extern unsigned short D_800E2294;

void func_800CC440(int a0, int a1, short *a2)
{
    int c;

    *(volatile short *)&a2[3] = D_800E2290;
    *(volatile short *)&a2[4] = D_800E2292;
    c = D_800E2294;
    *(volatile short *)&a2[2] = 0x224;
    *(volatile char *)((char *)a2 + 3) = 0x7F;
    a2[5] = c;
}
