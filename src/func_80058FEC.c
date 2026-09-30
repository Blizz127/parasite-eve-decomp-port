extern short *D_8009D048;
extern int D_8009D050;
extern unsigned int *D_8009D058;
extern int D_8009D064;
extern short *D_8009D07C;
extern int D_8009D044;
extern short D_800C0E48[];
extern unsigned int D_8009D05C[];
extern short D_800A1E00[];
extern signed char D_800C0E20;
extern signed char D_800C0E22;
extern unsigned char D_800BEEAC[];
extern unsigned char D_8009DE64[];

extern int func_80052F70(void);
extern void func_80055760(void);
extern int func_80058E44(void);
extern unsigned char *func_8005DB44(int);
extern void func_8004F448(void);

static inline int slot(int i)
{
    return (i >= 0 && i < D_8009D044) ? D_800A1E00[i] : 0;
}

static inline unsigned char *item(int v)
{
    unsigned char *res;

    if ((unsigned int)(v - 0x100) < 0x80) {
        res = (unsigned char *)((v << 5) + (int)D_800BEEAC);
        goto done;
    }
    if ((unsigned int)(v - 1) < 0xFF) {
        res = func_8005DB44(v - 1);
        goto done;
    }
    if ((unsigned int)(v - 0x200) < 9) {
        res = (unsigned char *)((v << 5) + (int)D_8009DE64);
        goto done;
    }
    res = 0;
done:
    return res;
}

static inline void swap(unsigned short *p, unsigned short *q)
{
    *p ^= *q;
    *q ^= *p;
    *p ^= *q;
}

static inline void swapi(int x, int y)
{
        swap((unsigned short *)&D_8009D048[x], (unsigned short *)&D_8009D048[y]);
        if (D_800C0E20 == x) {
            D_800C0E20 = y;
        } else if (D_800C0E20 == y) {
            D_800C0E20 = x;
        }
        if (D_800C0E22 == x) {
            D_800C0E22 = y;
        } else if (D_800C0E22 == y) {
            D_800C0E22 = x;
        }
}
int func_80058FEC(int a, int b, int c, int d)
{
    int r;
    int x;
    unsigned short *p;
    unsigned short *q;

    r = 1;
    D_8009D048 = D_800C0E48;
    D_8009D050 = func_80052F70();
    D_8009D058 = D_8009D05C;
    D_8009D064 = 2;
    if (a == 0x34 && c == a) {
        swap((unsigned short *)&D_8009D07C[b], (unsigned short *)&D_8009D07C[d]);
    done:
        func_80055760();
        goto out;
    }
    if (a == 0x33 && c == a) {
        swapi(slot(b), slot(d));
        goto done;
    }
    if (a == 0x34) {
        x = slot(d);
        if (func_80058E44() == 0) {
            goto fail;
        }
        if (item(D_8009D07C[b])[6] >= 0x13 && item(D_8009D07C[b])[6] < 0x16) {
            goto fail;
        }
        swap((unsigned short *)&D_8009D07C[b], (unsigned short *)&D_8009D048[x]);
        func_8004F448();
        goto done;
    } else {
        x = slot(b);
        if (func_80058E44() == 0) {
            goto fail;
        }
        if (item(D_8009D07C[d])[6] >= 0x13 && item(D_8009D07C[d])[6] < 0x16) {
            goto fail;
        }
        swap((unsigned short *)&D_8009D048[x], (unsigned short *)&D_8009D07C[d]);
        func_8004F448();
        goto done;
    }
fail:
    r = 0;
out:
    return r;
}
