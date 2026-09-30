typedef struct Font {
    unsigned char pad0[3];
    unsigned char cur;
    unsigned char pad4[8];
    unsigned char *data;
} Font;

extern Font D_80091A1C;

static inline unsigned char cur_attr(void)
{
    unsigned char *d = D_80091A1C.data;

    return (d + (d + D_80091A1C.cur)[29])[4];
}

static inline int find_slot(unsigned char *q, unsigned char sel)
{
    int i;
    int r;

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

unsigned char func_80039898(void)
{
    unsigned char *p;
    int i;
    unsigned char sel;

    sel = 0;
    p = D_80091A1C.data + 1;
    for (i = 0; i < p[2]; i++) {
        if ((p + i)[3] == 3) {
            sel = i;
            i = p[2];
        }
    }
    D_80091A1C.cur = find_slot(p + 27, sel);
    return cur_attr();
}
