extern unsigned char D_800A0ED5[];

int func_80042AD8(int a0) {
    unsigned char v;
    int r;

    v = D_800A0ED5[a0 * 1048];
    r = 0;
    if (v == 3 || v == 8 || v == 10) {
        r = 1;
    }
    return r;
}
