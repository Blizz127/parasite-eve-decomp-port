typedef struct Rec {
    int w[9];
} Rec;

extern signed char D_800C0E20[];
extern short *D_8009D048;
extern int D_8009D050;
extern int D_8009D040;
extern short D_800A1D9C[];
extern unsigned char D_8009DE64[];
extern Rec *func_80051098(void);
extern unsigned char *func_8005DB44(int);
extern void func_800512AC(int, int *);

static inline int get_sel(int i)
{
    int r;

    if (i >= 0 && i < D_8009D040) {
        r = D_800A1D9C[i];
    } else {
        r = 0;
    }
    return r;
}

void func_80059534(int sel)
{
    Rec *r;
    int local;
    int idx;
    int v;
    int w;
    unsigned char *res;

    r = func_80051098();
    r->w[0] = 2;
    idx = D_800C0E20[0];
    if (idx >= 0 && idx < D_8009D050) {
        v = D_8009D048[idx];
        w = v;
        if ((unsigned int)(v - 0x100) < 0x80) {
            res = (unsigned char *)((v << 5) + ((int)D_800C0E20 - 0x1F74));
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
    }
    res = 0;
done:
    r->w[1] = (int)res;
    local = get_sel(sel);
    D_800C0E20[0] = get_sel(sel);
    func_800512AC(2, &local);
}
