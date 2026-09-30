extern signed char D_800C0E22[];
extern short *D_8009D048;
extern int D_8009D050;
extern unsigned char D_8009DE64[];
extern unsigned char *func_8005DB44(int);
extern unsigned short *func_8005DC10(void);
extern void func_800515F8(int *);

int func_800579D4(int id, int a1)
{
    int n;
    unsigned char *p;
    unsigned char *res;
    int idx;
    int v;
    int w;
    int i;

    if (id == 6 || id == 0x13) {
        return a1;
    }
    if (id == 5) {
        func_800515F8(&n);
        return n / 3;
    }
    id = func_8005DC10()[id * 2 + 1];
    idx = D_800C0E22[0];
    if (idx >= 0 && idx < D_8009D050) {
        v = D_8009D048[idx];
        w = v;
        if ((unsigned int)(v - 0x100) < 0x80) {
            res = (unsigned char *)((v << 5) + ((int)D_800C0E22 - 0x1F76));
            goto done;
        }
        if ((unsigned int)(v - 1) < 0xFF) {
            res = func_8005DB44(v - 1);
            goto done;
        }
        if ((unsigned int)(w - 0x200) < 9) {
            res = (unsigned char *)((w << 5) + (int)D_8009DE64);
            goto done;
        }
        res = 0;
    done:
        p = res;
    } else {
        p = 0;
    }
    if (p != 0) {
        for (i = 0; i < p[0x14]; i++) {
            if ((p + i)[0x15] == 0xE) {
                break;
            }
        }
        if (i < p[0x14]) {
            id = (id * 2) / 3;
        }
    }
    return id;
}
