typedef struct Font {
    unsigned char pad0[3];
    unsigned char cur;
    unsigned char pad4[8];
    unsigned char *data;
} Font;

extern Font D_80091A1C;
extern unsigned char D_80091A1D;
extern unsigned char D_80091A1E;
extern unsigned char D_80091A20;
extern unsigned char D_8009EE22[];
extern int func_800389DC(int);

static inline unsigned char cur_attr(void)
{
    unsigned char *d = D_80091A1C.data;

    return (d + (d + D_80091A1C.cur)[29])[4];
}

static inline int find_pos(unsigned char cls)
{
    unsigned char *p;
    unsigned char *q;
    int i;
    int r;
    unsigned char sel;

    sel = 0;
    p = D_80091A1C.data + 1;
    for (i = 0; i < p[2]; i++) {
        if ((p + i)[3] == cls) {
            sel = i;
            i = p[2];
        }
    }
    q = p + 27;
    for (i = 0; i < q[0]; i++) {
        if ((q + i)[1] == sel) {
            goto found;
        }
    }
    r = 255;
    goto done;
found:
    r = i;
done:
    return r;
}

unsigned char func_80039310(void)
{
    unsigned char *k = &D_80091A1D;

    if (*k >= 70) {
        D_80091A20 = 1;
        return 255;
    }
    (*k)++;
    D_80091A1E = D_8009EE22[*k];
    D_80091A20 = 0;
    func_800389DC(D_80091A1E);
    D_80091A1C.cur = find_pos(2);
    return cur_attr();
}
