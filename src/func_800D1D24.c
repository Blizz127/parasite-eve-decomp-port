extern short *volatile D_800B1624;
extern int func_80077DC4(int a0);
extern void func_800679C4(int a0, int a1, int a2);

void func_800D1D24(int a0, int a1, int a2)
{
    int ang;
    int sign;
    int v;
    short *p;

    ang = (a2 << 10) / a1;
    sign = ((a2 & 1) << 1) - 1;
    v = func_80077DC4(ang) * a0 / 4096;
    p = D_800B1624;
    p[26] = -0x80;
    p[24] = -0x80;
    p = D_800B1624;
    p[27] = 0x80;
    p[25] = 0x80;
    func_800679C4(0, (short)(v * sign), 0);
}
