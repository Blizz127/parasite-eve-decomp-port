typedef struct {
    unsigned char b[8];
    unsigned short f8;
    unsigned short fA;
} H;

void func_800C7098(H *a0, int a1, int a2, int a3)
{
    unsigned char *p;
    int i;

    p = (unsigned char *)a0 + a0->f8;
    i = 0;
    while (i < a0->fA) {
        i++;
        p[0] = a1;
        p[1] = a2;
        p[2] = a3;
        p += 4;
    }
}
