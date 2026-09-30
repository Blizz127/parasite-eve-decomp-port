int func_800DFFB8(unsigned int a0, unsigned int a1, int a2)
{
    register unsigned int n asm("$4");
    register unsigned int m asm("$5");
    register unsigned int b asm("$8");
    register int d asm("$5");
    register int ad asm("$2");
    register int ad2 asm("$7");
    register int step asm("$3");
    register int small asm("$2");
    register int r asm("$2");

    n = a0 & 0xFFF;
    m = a1 & 0xFFF;
    b = m;
    d = (short)(m - n);
    ad = d;
    if (d < 0) {
        ad = -ad;
    }
    ad2 = (short)ad;
    step = a2;
    if ((short)a2 >= ad2) {
        r = b;
    } else {
        small = ad2 <= 0x800;
        if (d < 0) {
            step = -a2;
        }
        if (!small) {
            step = -step;
        }
        r = step + n;
    }
    return r & 0xFFF;
}
