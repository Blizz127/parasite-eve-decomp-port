typedef struct {
    unsigned char b[8];
    unsigned short f8;
    unsigned short fA;
} H;

void func_800C70EC(H *a0, int a1, int a2, int a3)
{
    unsigned char *p;
    register int i asm("$11");
    short v;
    short w;

    p = (unsigned char *)a0 + a0->f8;
    i = 0;
    while (i < a0->fA) {
        v = p[0];
        v = v + a1;
        if (v >= 0x100) {
            v = 0xFF;
        }
        if (v < 0) {
            v = 0;
        }
        p[0] = v;
        v = p[1];
        v = v + a2;
        if (v >= 0x100) {
            v = 0xFF;
        }
        if (v < 0) {
            v = 0;
        }
        w = p[2];
        w = w + a3;
        p[1] = v;
        if (w >= 0x100) {
            w = 0xFF;
        }
        if (w < 0) {
            w = 0;
        }
        i++;
        p[2] = w;
        p += 4;
    }
}
