void func_80192704(int a0, unsigned char *a1, unsigned char *a2)
{
    register unsigned char *base asm("$6");
    register unsigned char *p asm("$4");
    short v;
    unsigned short u;
    short s;

    base = a2;
    asm volatile("" : "=r"(base) : "0"(base));
    v = *(short *)(base + 0x10);
    if (v < 0x960) {
        p = base;
        asm volatile("" : "=r"(base) : "0"(base));
        *(short *)(base + 0x10) = (short)(v + 0x28);
    } else {
        p = base;
    }
    if (*(short *)(a1 + 2) >= 0x3D) {
        u = *(unsigned short *)(p + 0x12);
        u = (unsigned short)(u + 0x190);
        *(unsigned short *)(p + 0x12) = u;
        s = (short)u;
        if (s >= 0x1449) {
            *(short *)(p + 0x12) = 0x1448;
        }
    }
    if (*(short *)(a1 + 2) >= 0x6A) {
        a1[1] = 2;
    }
}
