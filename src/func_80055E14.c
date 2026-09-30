extern int D_8009D064;
extern unsigned int *D_8009D058;
extern int D_8009D050;
extern short *D_8009D048;
extern signed char D_800C0E20[];
extern unsigned char D_8009DE64[];
extern unsigned char *func_8005DB44(int);
extern int func_80057654(int);

void func_80055E14(void)
{
    int unused[2];
    int k;
    int i;
    signed char *slot;
    unsigned char *base;
    int v;
    int w;
    unsigned char *res;
    unsigned char *rec;
    unsigned int loaded;
    unsigned int out;
    unsigned int *bp;
    int off;
    register int bit asm("$6");

    for (k = 0; k < D_8009D064; k++) {
        D_8009D058[k] = 0;
    }
    if (D_8009D050 > 0) {
        i = 0;
        slot = D_800C0E20;
        base = (unsigned char *)((int)slot - 0x1F74);
        do {
            if (i != slot[0] && i != slot[2] && func_80057654(i) != 0) {
                if (i >= 0 && i < D_8009D050) {
                    v = D_8009D048[i];
                    w = v;
                    if ((unsigned int)(v - 0x100) < 0x80) {
                        res = (unsigned char *)((v << 5) + (int)base);
                        goto setrec;
                    }
                    if ((unsigned int)(v - 1) < 0xFF) {
                        res = func_8005DB44(v - 1);
                        goto setrec;
                    }
                    if ((unsigned int)(w - 0x200) < 9) {
                        res = (unsigned char *)((w << 5) + (int)D_8009DE64);
                        goto setrec;
                    }
                    res = 0;
                setrec:
                    rec = res;
                    goto have;
                }
                rec = 0;
            have:
                off = (i >> 5) << 2;
                __asm__ __volatile__("" : "=r"(off) : "0"(off));
                bp = (unsigned int *)(off + (int)D_8009D058);
                loaded = *bp;
                if (rec != 0) {
                    bit = i & 0x1F;
                    __asm__ __volatile__("" : "=r"(bit) : "0"(bit));
                    if ((rec[5] & 0xE0) == 0) {
                        out = loaded | (1 << bit);
                    } else {
                        out = loaded;
                    }
                    *bp = out;
                } else {
                    *bp = loaded;
                }
            }
            i++;
        } while (i < D_8009D050);
    }
}
