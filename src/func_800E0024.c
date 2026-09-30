short func_800E0024(int a0, int a1)
{
    short d;

    d = a0 - a1;
    if (d < 0) {
        d = -d;
    }
    if (d > 0x800) {
        d = 0x1000 - d;
    }
    return d;
}
