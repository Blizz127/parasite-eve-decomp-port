void func_8008F608(unsigned char *a0) {
    unsigned char *p;
    unsigned char c;
    int old;

    p = *(unsigned char **)a0;
    *(unsigned char **)a0 = p + 1;
    c = *p;
    *(unsigned short *)(a0 + 0x72) = c;
    if (c == 0) {
        *(unsigned short *)(a0 + 0x72) = 0x100;
    }
    {
        int val;
        int t;
        signed char *q = *(signed char **)a0;
        *(signed char **)a0 = q + 1;
        t = *q << 23;
        old = *(int *)(a0 + 0x44) & ~0xFFFF;
        val = (t - old) / *(unsigned short *)(a0 + 0x72);
        *(int *)(a0 + 0x44) = old;
        *(int *)(a0 + 0x48) = val;
    }
}
