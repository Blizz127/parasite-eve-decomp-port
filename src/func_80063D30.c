void func_80063D30(int *a0)
{
    int cur;
    int top;
    int h;

    cur = a0[18];
    top = a0[23];
    if (cur < top) {
        a0[23] = cur;
    } else {
        h = a0[14];
        if (cur >= top + h) {
            a0[23] = cur - h + 1;
        }
    }
}
