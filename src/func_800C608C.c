void func_800C608C(short a0, unsigned char *a1, unsigned char *a2) {
    if (a0 != 0x80) {
        int t;
        t = a1[0];
        t = t * a0;
        if (t > 0x7FFF) {
            t = 0x7FFF;
        }
        t = t >> 7;
        a2[0] = t;
        t = a1[1];
        t = t * a0;
        if (t > 0x7FFF) {
            t = 0x7FFF;
        }
        t = t >> 7;
        a2[1] = t;
        t = a1[2];
        t = t * a0;
        if (t > 0x7FFF) {
            t = 0x7FFF;
        }
        t = t >> 7;
        a2[2] = t;
    } else {
        a2[0] = a1[0];
        a2[1] = a1[1];
        a2[2] = a1[2];
    }
}
