unsigned int func_8006E3D4(signed char *a0)
{
    unsigned int acc;
    int i;
    int k;
    int c;
    int d;

    acc = 0;
    i = 0;
    k = 5;
    do {
        c = *a0;
        d = c - 0x30;
        if ((unsigned int)d < 10) {
            c = d;
        } else {
            c = c - 0x57 - (c >= 0x6B) - (c >= 0x72) - (c >= 0x79);
        }
        acc |= c << ((k - i) * 5 + 2);
        i++;
        a0++;
    } while (i < 6);
    return acc;
}
