typedef struct Obj {
    int f0;
    unsigned char pad4[0x93];
    unsigned char b97;
    unsigned char b98;
    unsigned char b99;
    short h9A;
    unsigned short f9C;
    unsigned char b9E;
    unsigned char b9F;
    unsigned char padA0[0x1A];
    short fBA;
} Obj;

extern int D_8009CDDC;
extern unsigned char D_800BEA40[];
extern void func_8003CEF8(Obj *, int);
extern void func_8003CCB0(Obj *, int);
extern void func_8003C2E0(Obj *, short, int);
extern void func_8003B144(Obj *);
extern void func_8003B708(Obj *, short);
extern short func_8003C818(Obj *);
extern short func_8003C638(Obj *);
extern void func_8003B97C(Obj *, void *);
extern void func_8003C0B4(Obj *, short, unsigned char, unsigned char, unsigned char);
extern void func_8003BCE0(Obj *, int, short);

void func_8003AF14(Obj *o, int arg)
{
    if (o->f0 == 0) {
        return;
    }
    if (o->fBA == 0) {
        return;
    }
    if (o->f9C & 0x800) {
        func_8003CEF8(o, 0);
        func_8003CCB0(o, 0);
        o->f9C &= ~0x800;
    }
    if (o->f9C & 0x10) {
        func_8003C2E0(o, o->h9A, arg);
    }
    if (o->b9E == 1) {
        func_8003B144(o);
    }
    if (o->f9C & 0x20) {
        func_8003B708(o, D_8009CDDC);
        o->b9F = D_8009CDDC == 0;
        o->f9C = (o->f9C & ~0x20) | 0x40;
    } else if (o->f9C & 0x40) {
        func_8003B708(o, o->b9F);
        o->f9C &= ~0x40;
    }
    if (o->f9C & 2) {
        if (func_8003C818(o) != 0) {
            o->f9C = (o->f9C & ~2) | 0x200;
        }
        if (o->f9C & 8) {
            func_8003C0B4(o, o->h9A, o->b97, o->b98, o->b99);
        }
    } else if (o->f9C & 4) {
        if (func_8003C638(o) != 0) {
            o->f9C &= ~4;
        }
        if (o->f9C & 8) {
            func_8003C0B4(o, o->h9A, o->b97, o->b98, o->b99);
        }
    } else if (o->f9C & 8) {
        func_8003B97C(o, D_800BEA40);
        func_8003C0B4(o, o->h9A, o->b97, o->b98, o->b99);
    } else if (o->f9C & 1) {
        func_8003B97C(o, D_800BEA40);
        func_8003BCE0(o, 0, D_8009CDDC);
    }
}
