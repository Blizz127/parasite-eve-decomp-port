extern int func_80079FB4(int a0, int a1);

void func_800DFE94(int *a0, int *a1, short *a2)
{
    int dx;
    int dz;
    int acc;
    int r;
    int s;
    int t;

    dx = (a0[0] - a1[0]) >> 16;
    dz = (a0[2] - a1[2]) >> 16;
    acc = dx * dx + dz * dz;
    r = 0;
    s = 30;
    do {
        t = ((r << 2) + 1) << s;
        r = r << 1;
        if (acc >= t) {
            acc -= t;
            r |= 1;
        }
        s -= 2;
    } while (s >= 0);
    a2[0] = func_80079FB4(a0[1] - a1[1], r << 16) & 0xFFF;
    a2[1] = func_80079FB4(a1[0] - a0[0], a1[2] - a0[2]) & 0xFFF;
    a2[2] = 0;
}
