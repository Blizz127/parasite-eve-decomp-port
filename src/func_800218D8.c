typedef struct {
    unsigned short f0;
    unsigned short f2;
    unsigned short f4;
    unsigned short f6;
    int f8;
    unsigned int c0 : 10;
    unsigned int c1 : 10;
    unsigned int c2 : 2;
    unsigned int pad_c : 10;
    unsigned int d0 : 4;
    unsigned int d1 : 2;
    unsigned int d2 : 2;
    unsigned int d3 : 1;
    unsigned int d4 : 1;
    unsigned int d5 : 1;
    unsigned int d6 : 1;
    unsigned int d7 : 1;
    unsigned int d8 : 2;
    unsigned int d9 : 1;
    unsigned short d10 : 1;
    unsigned int d11 : 1;
} Obj;

typedef struct {
    unsigned char pad0[0x68];
    Obj *f68;
} Rec;

extern Rec *D_8009D278;
extern Obj D_800A76D8;

void func_800218D8(void) {
    Obj *o = D_8009D278->f68;

    o->f0 = D_800A76D8.f0;
    o->f2 = D_800A76D8.f2;
    o->f4 = D_800A76D8.f4;
    o->f6 = D_800A76D8.f6;
    o->f8 = D_800A76D8.f8;
    o->c0 = D_800A76D8.c0;
    o->c1 = D_800A76D8.c1;
    o->c2 = D_800A76D8.c2;
    o->d0 = D_800A76D8.d0;
    o->d1 = D_800A76D8.d1;
    o->d2 = D_800A76D8.d2;
    o->d3 = D_800A76D8.d3;
    o->d4 = D_800A76D8.d4;
    o->d5 = D_800A76D8.d5;
    o->d6 = D_800A76D8.d6;
    o->d7 = D_800A76D8.d7;
    o->d8 = D_800A76D8.d8;
    o->d9 = D_800A76D8.d9;
    o->d10 = D_800A76D8.d10;
    o->d11 = D_800A76D8.d11;
}
