int func_800DFC80(int *a0, int *a1)
{
    int dx;
    int dy;
    int dz;
    int acc;
    int r;
    int s;
    int t;
    int acc2;
    int r2;
    int s2;
    int t2;

    dx = (a0[0] - a1[0]) >> 16;
    dz = (a0[2] - a1[2]) >> 16;
    dy = (a0[1] - a1[1]) >> 16;
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
    acc2 = dy * dy + r * r;
    r2 = 0;
    s2 = 30;
    do {
        t2 = ((r2 << 2) + 1) << s2;
        r2 = r2 << 1;
        if (acc2 >= t2) {
            acc2 -= t2;
            r2 |= 1;
        }
        s2 -= 2;
    } while (s2 >= 0);
    return r2;
}
