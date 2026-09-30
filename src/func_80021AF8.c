typedef struct {
    unsigned int a0 : 10;
    unsigned int a1 : 10;
    unsigned int a2 : 8;
    unsigned int a3 : 4;
    unsigned int b0 : 1;
    unsigned int b1 : 1;
    unsigned int b2 : 1;
    unsigned int b3 : 1;
    unsigned int b4 : 1;
    unsigned int b5 : 4;
    unsigned int b6 : 1;
    unsigned int b7 : 1;
    unsigned int b8 : 1;
    unsigned int b9 : 1;
    unsigned int b10 : 1;
    unsigned int b11 : 1;
    unsigned int b12 : 1;
    unsigned short b13 : 1;
    unsigned int b14 : 1;
} Cfg;

typedef struct {
    unsigned char pad0[0x6C];
    Cfg *f6C;
} Rec;

extern Rec *D_8009D278;
extern Cfg D_800A76D8;

void func_80021AF8(void) {
    Cfg *c = D_8009D278->f6C;

    c->a0 = D_800A76D8.a0;
    c->a1 = D_800A76D8.a1;
    c->a2 = D_800A76D8.a2;
    c->a3 = D_800A76D8.a3;
    c->b0 = D_800A76D8.b0;
    c->b1 = D_800A76D8.b1;
    c->b2 = D_800A76D8.b2;
    c->b3 = D_800A76D8.b3;
    c->b4 = D_800A76D8.b4;
    c->b5 = D_800A76D8.b5;
    c->b6 = D_800A76D8.b6;
    c->b7 = D_800A76D8.b7;
    c->b8 = D_800A76D8.b8;
    c->b9 = D_800A76D8.b9;
    c->b10 = D_800A76D8.b10;
    c->b11 = D_800A76D8.b11;
    c->b12 = D_800A76D8.b12;
    c->b13 = D_800A76D8.b13;
    c->b14 = D_800A76D8.b14;
}
