extern unsigned int *D_8009D2C8;

void func_8008A8CC(unsigned char *a0, unsigned int a1)
{
    unsigned int i;
    unsigned int newv;
    unsigned int *q;
    unsigned int one;

    if (a1 < 0x18) {
        i = 0;
        newv = 0x18;
        q = D_8009D2C8;
        one = 1;
        a0 += 0xF0;
        do {
            if (*(unsigned int *)a0 == a1) {
                *(unsigned int *)a0 = newv;
                q[5] &= ~(one << i);
            }
            i++;
            a0 += 0x11C;
        } while (i < 0x18);
    }
}
