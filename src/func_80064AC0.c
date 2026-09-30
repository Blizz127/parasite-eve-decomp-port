extern int D_8009D16C;
extern signed char D_800A3060[];

void func_80064AC0(int *a0)
{
    signed char *q;
    int idx;
    int t;
    int lim;

    if (a0 == 0) {
        return;
    }
    idx = a0[28];
    if (idx < 0) {
        return;
    }
    q = &D_800A3060[idx * 4];
    a0[17] = q[0];
    if (D_8009D16C == 0 && (a0[25] & 0x20) == 0) {
        return;
    }
    t = q[1];
    lim = a0[22];
    a0[18] = t;
    if (t >= lim) {
        a0[18] = lim - 1;
    }
    if (a0[26] != 0 && a0[17] == 1 && a0[18] == a0[22] - 1) {
        a0[17] = 0;
    }
    a0[23] = q[2];
}
