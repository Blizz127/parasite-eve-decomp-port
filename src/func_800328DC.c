typedef struct Sprt {
    unsigned int tag;
    unsigned char r0, g0, b0, code;
    short x0, y0;
    unsigned char u0, v0;
    unsigned short clut;
    short w, h;
} Sprt;

typedef struct Ent {
    unsigned int tp[2];
    Sprt sp;
} Ent;

extern int D_8009CDDC;
extern Ent D_8009E460[];
extern unsigned char *D_800B0E38[];
extern void func_80077AC4(unsigned char *, Ent *);

signed char func_800328DC(Ent *tbl, short x, short y, short v, short mode)
{
    unsigned char buf[8];
    signed char n;
    signed char m;
    short q;

    n = 0;
    while (q = v / 10, buf[n] = v - q * 10, (v = q) != 0) {
        n++;
    }
    m = n;
    x -= n * 6;
    for (; n >= 0; n--) {
        Ent *e = (Ent *)(n * 28 + (int)tbl);
        unsigned char c = buf[n];
        (&e->sp)->v0 = 242;
        (&e->sp)->x0 = x;
        (&e->sp)->y0 = y;
        (&e->sp)->u0 = c << 3;
        if (mode == 1) {
            (&e->sp)->r0 = D_8009E460[D_8009CDDC].sp.r0 + 40;
            (&e->sp)->g0 = D_8009E460[D_8009CDDC].sp.g0;
            (&e->sp)->b0 = D_8009E460[D_8009CDDC].sp.b0;
        }
        func_80077AC4(D_800B0E38[D_8009CDDC] + 16, e);
        x += 6;
    }
    return m;
}
