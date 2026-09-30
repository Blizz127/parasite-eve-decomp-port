typedef struct Obj {
    int f0;
    unsigned char pad4[0x88];
    signed char c8C;
    signed char c8D;
    unsigned char b8E;
    unsigned char b8F;
    unsigned char b90;
    unsigned char b91;
    unsigned char b92;
    unsigned char b93;
    unsigned char b94;
    unsigned char b95;
    unsigned char b96;
    unsigned char pad97[5];
    unsigned short f9C;
    unsigned char b9E;
    unsigned char pad9F[0x1B];
    short fBA;
} Obj;

extern int D_8009CDA0;
extern short D_8009CDDC[];
extern unsigned char D_800BEA40[];
extern void func_8003CEF8(Obj *, int);
extern void func_8006698C(Obj *);
extern void func_8003CCB0(Obj *, int);
extern void func_8003CAEC(Obj *, int, int, int);
extern void func_8003B97C(Obj *, void *);
extern void func_8003BCE0(Obj *, int, int);

int func_8003C638(Obj *o)
{
    int c;
    unsigned char s0;
    unsigned char s1;
    unsigned char s2;

    if (o->f0 == 0) {
        return 1;
    }
    if (o->fBA == 0) {
        return 1;
    }
    c = o->c8C;
    if (c == 0) {
        o->c8C = -1;
        func_8003CEF8(o, 0);
        func_8006698C(o);
        func_8003CCB0(o, 0);
        o->f9C |= 0x820;
        return 1;
    }
    if (c < 0) {
        o->b9E = 1;
        o->f9C &= ~0x200;
        func_8003CEF8(o, 1);
        func_8003CCB0(o, 1);
        s0 = o->b90;
        s1 = o->b91;
        s2 = o->b92;
        o->b94 = 0;
        o->b95 = 0;
        o->b96 = 0;
        func_8003CAEC(o, 0, 0, 0);
        o->b90 = s0;
        o->b91 = s1;
        o->b92 = s2;
        o->c8C = o->c8D;
    } else if (c == o->c8D - 1) {
        func_8003CEF8(o, 1);
        func_8003CCB0(o, 1);
    }
    D_8009CDA0 = (o->b96 << 16) | (o->b95 << 8) | o->b94;
    func_8003B97C(o, D_800BEA40);
    if (!(o->f9C & 8)) {
        func_8003BCE0(o, 0, D_8009CDDC[0]);
    }
    D_8009CDA0 = 0x808080;
    o->b94 += o->b8E;
    o->b95 += o->b8F;
    o->b96 += o->b93;
    o->c8C--;
    return 0;
}
