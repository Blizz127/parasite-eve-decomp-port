extern int func_80078004(int a0);

int func_800CFE94(short *a0, short *a1)
{
    int dx;
    int dy;
    int dz;
    int r;

    dx = a1[0] - a0[0];
    dy = a1[1] - a0[1];
    dz = a1[2] - a0[2];
    r = func_80078004(dx * dx + dy * dy + dz * dz);
    if (r == 0) {
        r = 1;
    }
    return r;
}
