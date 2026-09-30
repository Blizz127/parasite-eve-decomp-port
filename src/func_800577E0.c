extern unsigned char *func_8005DB44(int);

void func_800577E0(int a0, int a1)
{
    unsigned char *p;
    int v;

    p = func_8005DB44(a0 - 1);
    if (p != 0) {
        v = p[5] & 0xBF;
        p[5] = v;
        if (a1 == 0) {
            v |= 0x40;
        }
        p[5] = v;
    }
}
