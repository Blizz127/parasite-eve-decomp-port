extern int D_8009D0A0;
extern unsigned char *(*D_8009D0B4)(int);

int func_8005B124(short *a, short *b)
{
    unsigned char *p;
    unsigned char *q;
    int x;
    int y;

    x = 0;
    y = 0;
    p = D_8009D0B4(*a);
    q = D_8009D0B4(*b);
    switch (D_8009D0A0) {
    case 0:
        x = p[7] + *(short *)(p + 0xE);
        y = q[7] + *(short *)(q + 0xE);
        break;
    case 1:
        x = p[8] + *(short *)(p + 0x10);
        y = q[8] + *(short *)(q + 0x10);
        break;
    case 2:
        x = p[9] + *(short *)(p + 0x12);
        y = q[9] + *(short *)(q + 0x12);
        break;
    }
    if (y < x) {
        return -1;
    }
    if (x < y) {
        return 1;
    }
    if (p[4] > q[4]) {
        goto one;
    }
    return -(p[4] < q[4]);
one:
    return 1;
}
