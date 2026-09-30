extern int func_80079FB4(int a0, int a1);
extern int func_80078004(int a0);

void func_800CFAA8(short *a0, short *a1, short *a2)
{
    int dx;
    int dz;
    int d;

    dz = a1[2] - a0[2];
    dx = a1[0] - a0[0];
    a2[1] = -func_80079FB4(dz, dx) + 0x400;
    d = func_80078004(dx * dx + dz * dz);
    a2[0] = -func_80079FB4(a1[1] - a0[1], d);
    a2[2] = 0;
    a2[0] &= 0xFFF;
    a2[1] &= 0xFFF;
}
