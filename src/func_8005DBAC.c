extern int D_800A803C;

int func_8005DBAC(int a0)
{
    register int v asm("$3");
    register int *base asm("$4");
    int sh;
    int *p;
    int start;

    v = a0;
    if (v < 0) {
        v = 0;
    } else if (v >= 0x63) {
        v = 0x62;
    }
    base = &D_800A803C;
    sh = v * 24;
    p = base - 5;
    start = *base;
    return start + (sh + (int)p);
}
