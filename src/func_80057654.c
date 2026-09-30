extern short *D_8009D048;
extern int D_8009D050;
extern unsigned char D_800BEEAC[];
extern unsigned char D_8009DE64[];
extern unsigned char D_800C0E48[];
extern signed char D_800C0E20[];
extern unsigned char *func_8005DB44(int);
extern int func_8005415C(int);

int func_80057654(int idx)
{
    int unused[2];
    register unsigned char *rv asm("$2");
    register unsigned char *p asm("$4");
    register int s2 asm("$18");
    register int eight asm("$19");
    int v;
    int w;
    int i;
    int count;

    if (idx >= 0 && idx < D_8009D050) {
        v = D_8009D048[idx];
        w = v;
        if ((unsigned int)(v - 0x100) < 0x80U) {
            rv = (unsigned char *)((v << 5) + (int)D_800BEEAC);
            goto join;
        }
        if ((unsigned int)(v - 1) < 0xFFU) {
            rv = func_8005DB44(v - 1);
            goto join;
        }
        if ((unsigned int)(w - 0x200) < 9U) {
            rv = (unsigned char *)((w << 5) + (int)D_8009DE64);
            goto join;
        }
        rv = 0;
    join:
        p = rv;
    } else {
        p = 0;
    }
    if (p == 0) {
        return 1;
    }
    s2 = 0;
    if ((p[5] & 0x40) == 0) {
        if (D_8009D048 != (short *)D_800C0E48 || idx != D_800C0E20[0]) {
            s2 = 1;
        }
    }
    if (p != 0 && p[6] == 8) {
        i = 0;
        count = 0;
        eight = 8;
        if (D_8009D050 > 0) {
            do {
                if (func_8005415C(i) == eight) {
                    count++;
                }
                i++;
            } while (i < D_8009D050);
        }
        if (count < 2) {
            s2 = 0;
        } else {
            s2 &= 1;
        }
    }
    return s2;
}
