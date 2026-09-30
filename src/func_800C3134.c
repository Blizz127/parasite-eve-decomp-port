void func_800C3134(unsigned char *a0, unsigned int a1, unsigned char *a2) {
    unsigned int acc = 0;
    unsigned int one = 0x1000;
    unsigned int end = 0xFF;
    unsigned char *p = a0;

    for (;;) {
        unsigned int d = p[3];
        unsigned char *next;
        acc += d;
        next = p + 4;
        if (a1 < acc) {
            unsigned int t = ((acc - a1) << 12) / d;
            unsigned int u = one - t;
            unsigned int m0, m1, m2, m3, m4, m5;
            m0 = u * p[4];
            m1 = t * p[0];
            m2 = u * p[5];
            m3 = t * p[1];
            m4 = u * p[6];
            m5 = t * p[2];
            a2[0] = (m0 + m1) >> 12;
            a2[1] = (m2 + m3) >> 12;
            a2[2] = (m4 + m5) >> 12;
            return;
        }
        if (p[7] != end) {
            p = next;
            continue;
        }
        a2[0] = p[4];
        a2[1] = p[5];
        a2[2] = p[6];
        return;
    }
}
