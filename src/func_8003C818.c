typedef struct Obj {
    void *f0;
    unsigned char pad4[0x88];
    signed char f8C;
    signed char f8D;
    unsigned char dr;
    unsigned char dg;
    unsigned char r;
    unsigned char g;
    unsigned char b;
    unsigned char db;
    unsigned char cr;
    unsigned char cg;
    unsigned char cb;
    unsigned char pad97[5];
    unsigned short f9C;
    unsigned char f9E;
    unsigned char pad9F[0x1B];
    short fBA;
} Obj;

extern unsigned int D_8009CDA0;
extern short D_8009CDDC;
extern unsigned char D_800BEA40[];
extern void func_8003CCB0(Obj *, int);
extern void func_8003CEF8(Obj *, int);
extern void func_8003B97C(Obj *, unsigned char *);
extern void func_8003BCE0(Obj *, int, int);

int func_8003C818(Obj *o)
{
    int t;

    if (o->f0 == 0 || o->fBA == 0) {
        return 1;
    }
    t = o->f8C;
    if (t == 0) {
        o->f8C = -1;
        D_8009CDA0 = 0x808080;
        return 1;
    }
    if (t == 1) {
        o->f9E = 0;
    } else if (t < 0) {
        o->f8C = o->f8D;
        func_8003CCB0(o, 1);
        func_8003CEF8(o, 1);
        if (o->r > 0x80) {
            o->r = 0x80;
        }
        if (o->g > 0x80) {
            o->g = 0x80;
        }
        if (o->b > 0x80) {
            o->b = 0x80;
        }
        o->dr = o->r / o->f8D;
        o->dg = o->g / o->f8D;
        o->db = o->b / o->f8D;
        o->cr = o->r;
        o->cg = o->g;
        o->cb = o->b;
    } else if (t == o->f8D - 1) {
        func_8003CCB0(o, 1);
        func_8003CEF8(o, 1);
    } else {
        o->cr -= o->dr;
        if (o->cr > 0x80) {
            o->cr = 0;
        }
        o->cg -= o->dg;
        if (o->cg > 0x80) {
            o->cg = 0;
        }
        o->cb -= o->db;
        if (o->cb > 0x80) {
            o->cb = 0;
        }
    }
    D_8009CDA0 = (o->cb << 16) | (o->cg << 8) | o->cr;
    func_8003B97C(o, D_800BEA40);
    if (!(o->f9C & 8)) {
        func_8003BCE0(o, 0, D_8009CDDC);
    }
    D_8009CDA0 = 0x808080;
    o->f8C--;
    return 0;
}
