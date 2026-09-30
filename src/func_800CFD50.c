void func_800CFD50(unsigned short *a0, unsigned short *a1, unsigned short speed)
{
    volatile short slot;

    {
        register int dm asm("$3");
        register int diff asm("$2");
        register int ad asm("$7");
        int s;
        int ss;
        int prod;
        int q;
        int qs;

        diff = a0[0];
        dm = a1[0];
        ad = diff & 0xFFF;
        dm = dm & 0xFFF;
        s = -1;
        if ((unsigned int)dm < (unsigned int)ad) {
            s = 1;
        }
        diff = ad - dm;
        ad = diff;
        if (diff < 0) {
            ad = -ad;
        }
        if (ad > 0x800) {
            s = -(short)s;
            ad = 0x1000 - ad;
        }
        prod = (short)ad * speed;
        slot = ad;
        ss = (short)s;
        if (prod < 0) {
            prod += 0xFFF;
        }
        __asm__ __volatile__("" : "=r"(prod) : "0"(prod));
        q = prod >> 12;
        qs = (short)q;
        slot = q;
        a1[0] = a1[0] + qs * ss;
    }
    {
        register int dm asm("$3");
        register int diff asm("$2");
        register int ad asm("$4");
        int s;
        int prod;
        int q;
        int qs;

        diff = a0[1];
        dm = a1[1];
        ad = diff & 0xFFF;
        dm = dm & 0xFFF;
        __asm__ __volatile__("" : "=r"(ad) : "0"(ad));
        s = -1;
        if ((unsigned int)dm < (unsigned int)ad) {
            s = 1;
        }
        diff = ad - dm;
        ad = diff;
        if (diff < 0) {
            ad = -ad;
        }
        if (ad > 0x800) {
            s = -(short)s;
            ad = 0x1000 - ad;
        }
        prod = (short)ad * speed;
        slot = ad;
        s = (short)s;
        if (prod < 0) {
            prod += 0xFFF;
        }
        __asm__ __volatile__("" : "=r"(prod) : "0"(prod));
        q = prod >> 12;
        qs = (short)q;
        slot = q;
        a1[1] = a1[1] + qs * s;
    }
}
