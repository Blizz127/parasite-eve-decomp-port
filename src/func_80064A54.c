extern int D_8009D16C;
extern unsigned char D_800A3060[];

void func_80064A54(int *a0)
{
    unsigned int idx;
    unsigned char *q;

    if (D_8009D16C != 0 || (a0[25] & 0x20) != 0) {
        idx = a0[28];
        if (idx < 0x48) {
            q = &D_800A3060[idx * 4];
            q[0] = a0[17];
            q[1] = a0[18];
            q[2] = a0[23];
        }
    }
}
