extern int *func_8005DB8C(int);

void func_8005B91C(int a0, int val, int *outIdx, int *outFrac)
{
    int *t;
    int i;
    int step;
    int frac;

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
    if (i < 0x62 && t[i + 1] != t[i]) {
        frac = ((val - t[i]) * 49) / (t[i + 1] - t[i]);
    } else {
        frac = 0;
    }
    if (frac >= 0x31) {
        frac = 0x30;
    }
    if (outIdx != 0) {
        *outIdx = i;
    }
    if (outFrac != 0) {
        *outFrac = (i < 0x62) ? frac : 0x30;
    }
}
