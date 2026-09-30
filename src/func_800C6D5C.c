extern short D_800F3420;

void func_800C6D5C(unsigned char *m, unsigned char du, unsigned char dv)
{
    unsigned short i;
    unsigned char *p;
    unsigned char *q;

    p = m + 0x10;
    if (*(unsigned short *)(m + 0xC) != du || *(unsigned short *)(m + 0xE) != dv) {
        *(unsigned short *)(m + 0xC) = du;
        *(unsigned short *)(m + 0xE) = dv;
        for (i = 0; i < *(unsigned short *)(m + 0) + *(unsigned short *)(m + 2);) {
            i++;
            p[6] += du;
            p[8] += du;
            p[10] += du;
            p[7] += dv;
            p[9] += dv;
            p[11] += dv;
            p += 12;
        }
        q = p;
        for (i = 0; i < *(unsigned short *)(m + 4) + *(unsigned short *)(m + 6);) {
            i++;
            q[8] += du;
            q[10] += du;
            q[12] += du;
            q[14] += du;
            q[9] += dv;
            q[11] += dv;
            q[13] += dv;
            q[15] += dv;
            q += 16;
        }
    }
    D_800F3420 = 0;
}
