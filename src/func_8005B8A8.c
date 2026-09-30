int func_8005B8A8(int key, int *tbl)
{
    int idx;
    int step;
    int *p;
    int r;

    idx = 0x40;
    step = 0x20;
    do {
        p = (int *)((idx << 2) + (int)tbl);
        if (key >= *p) {
            idx += step;
        } else if (key < p[-1]) {
            idx -= step;
        } else {
            step = 0;
        }
        step >>= 1;
    } while (step != 0);
    idx--;
    r = 0x62;
    if (idx < 0x63) {
        r = idx;
    }
    return r;
}
