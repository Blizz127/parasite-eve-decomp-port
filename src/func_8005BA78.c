extern int *func_8005DB8C(int);

void func_8005BA78(int a0, int val, int *outRem, int *outFrac)
{
    int *t;
    int i;
    int step;
    int frac;
    int rem;

    t = func_8005DB8C(a0);
    i = 0x40;
    step = 0x20;
    do {
        if (t[i] <= val) {
            i += step;
        } else if (t[i - 1] > val) {
            i -= step;
        } else {
            step = 0;
        }
        step >>= 1;
    } while (step != 0);
    i--;
    i = (i > 0x62) ? 0x62 : i;
    rem = (i < 0x62) ? t[i + 1] - val : 0;
    if (i < 0x62 && t[i + 1] != t[i]) {
        frac = ((val - t[i]) * 49) / (t[i + 1] - t[i]);
    } else {
        frac = 0;
    }
    if (outRem != 0) {
        *outRem = rem;
    }
    if (outFrac != 0) {
        *outFrac = (i < 0x62) ? frac : 0x30;
    }
}
