int func_800DFE20(int *a0, int *a1)
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
    return r;
}
