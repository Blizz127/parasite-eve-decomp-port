short func_800CFCF4(unsigned int a0, unsigned int a1, short *a2)
{
    int s;
    register int d asm("$2");
    register int ad asm("$3");

    a0 &= 0xFFF;
    a1 &= 0xFFF;
    if (a1 < a0) {
        s = 1;
    } else {
        s = -1;
    }
    d = a0 - a1;
    ad = d;
    if (d < 0) {
        ad = -ad;
    }
    if (ad > 0x800) {
        s = -(short)s;
        ad = 0x1000 - ad;
    }
    if (a2 != 0) {
        *a2 = ad;
    }
    return s;
}
