void func_80063D78(int *a0, int a1, int a2)
{
    int top;

    if (a1 < 0) {
        a1 = 0;
    } else if (a1 >= a0[21]) {
        a1 = a0[21] - 1;
    }
    if (a2 < 0) {
        a2 = 0;
    } else if (a2 >= a0[22]) {
        a2 = a0[22] - 1;
    }
    a0[17] = a1;
    a0[18] = a2;
    top = a0[23];
    if (a2 < top) {
        a0[23] = a2;
    } else if (a2 >= top + a0[14]) {
        a0[23] = a2 - a0[14] + 1;
    }
}
