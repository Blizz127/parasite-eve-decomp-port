extern unsigned char D_800C0EAC[];
extern unsigned char D_800C0E48[];
extern unsigned char D_800C0E20[];
extern unsigned char D_800C0E22[];
extern unsigned char D_800BEEAC[];
extern unsigned char D_8009DE64[];
extern unsigned char D_8009D05C[];
extern short *D_8009D048;
extern int D_8009D050;
extern unsigned char *D_8009D058;
extern int D_8009D064;
extern int func_80052F70(void);
extern unsigned char *func_8005DB44(int);

int func_8005BF44(int id)
{
    unsigned char *p;
    short *q;
    int tag;
    int t;
    int v;
    int w;
    register int k asm("$3");
    unsigned char *res;

    p = D_800C0EAC;
    while (p < D_800C0EAC + 0x1000 && p[4] != id) {
        p += 0x20;
    }
    if (p < D_800C0EAC + 0x1000) {
        D_8009D048 = (short *)(D_800C0EAC - 0x64);
        D_8009D050 = func_80052F70();
        D_8009D058 = D_8009D05C;
        D_8009D064 = 2;
        tag = ((p - D_800C0EAC) >> 5) + 0x100;
        q = D_8009D048;
        while (q < D_8009D048 + D_8009D050 && *q != tag) {
            q++;
        }
        if (q < D_8009D048 + D_8009D050) {
            t = q - D_8009D048;
            goto d1;
        }
        t = -1;
    d1:
        id = t;
        if (id >= 0) {
            if (id < D_8009D050) {
                short *lst = D_8009D048;
                __asm__ __volatile__("" : "=r"(lst) : "0"(lst));
                v = lst[id];
                w = v;
                if ((unsigned int)(v - 0x100) < 0x80) {
                    res = (unsigned char *)((v << 5) + (int)D_800BEEAC);
                    goto done;
                }
                if ((unsigned int)(v - 1) < 0xFF) {
                    res = func_8005DB44(v - 1);
                    goto done;
                }
                if ((unsigned int)(w - 0x200) < 9) {
                    __asm__ __volatile__("");
                    res = (unsigned char *)((w << 5) + (int)D_8009DE64);
                    goto done;
                }
            }
            res = 0;
        done:
            k = 0;
            if (res != 0) {
                k = res[6];
            }
            if (k == 0) {
                goto neg;
            }
            if (k < 9) {
                goto arm_a;
            }
            if (k == 9) {
                goto arm_b;
            }
            id = -1;
            goto out;
        arm_a:
            D_800C0E20[0] = id;
            goto out;
        arm_b:
            D_800C0E22[0] = id;
            goto out;
        neg:
            id = -1;
        out:
            ;
        }
    }
    return (unsigned int)id >> 31;
}
