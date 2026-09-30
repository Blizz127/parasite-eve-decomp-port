typedef struct {
    unsigned int tag;
    unsigned char r0, g0, b0, code;
    short x0, y0;
    unsigned char u0, v0;
    unsigned short clut;
    short w, h;
} Sprt;

typedef struct {
    unsigned int tag;
    unsigned char r0, g0, b0, code;
    short x0, y0;
    short w, h;
} Tile;

typedef struct {
    unsigned int tp[2];
    Sprt sp;
} SEnt;

typedef struct {
    unsigned int tp[2];
    Tile t;
} TEnt;

typedef struct {
    unsigned int b0 : 8;
    unsigned int b8 : 8;
    unsigned int b16 : 4;
    unsigned int b20 : 1;
    unsigned int b21 : 1;
    unsigned int b22 : 3;
    unsigned int b25 : 1;
    unsigned int b26 : 6;
} Flags;

typedef struct {
    unsigned char active;
    unsigned char pad1[3];
    int f4;
    unsigned char type;
    unsigned char f9;
    unsigned char padA[2];
    Flags fl;
    short val;
    short pos[4];
    unsigned char dig[5][6];
} Pop;

extern Pop D_800BCEA8[];
extern unsigned char D_8009CEA0;
extern unsigned char D_8009CED0;
extern int D_8009CED4;
extern int D_8009CE90;
extern unsigned short D_80091680[];
extern SEnt D_8009EC70[];
extern TEnt D_8009ECA8[];

extern void func_80077C84(void *, int, int, int);
extern void func_80077C04(Sprt *);
extern void func_80077C44(Tile *);
extern int func_80077CB4(void *, void *);
extern void func_800719E4(int) __attribute__((noreturn));
extern void func_80077B34(Sprt *, int);
extern int func_80077A64(int, int, int, int);
extern void func_80077B04(Tile *, int);

void func_800371B0(int a0)
{
    unsigned char i;
    SEnt *pe;
    Sprt *ps;
    TEnt *pt;
    Tile *tl;

    for (i = 0; i < 4; i++) {
        D_800BCEA8[i].fl.b0 = 0;
        D_800BCEA8[i].fl.b8 = 0;
        D_800BCEA8[i].active = 0;
        D_800BCEA8[i].f4 = 0;
        D_800BCEA8[i].type = 0;
        D_800BCEA8[i].f9 = 0;
        D_800BCEA8[i].fl.b16 = 0;
        D_800BCEA8[i].fl.b20 = 0;
        D_800BCEA8[i].fl.b21 = 0;
        D_800BCEA8[i].fl.b22 = 0;
        D_800BCEA8[i].fl.b25 = 0;
        D_800BCEA8[i].val = -1;
    }
    D_8009CEA0 = 0;
    D_8009CED0 = 0;
    D_8009CED4 = 0;
    D_8009CE90 = a0;
    for (i = 0; i < 2; i++) {
        pe = &D_8009EC70[i];
        func_80077C84(pe, 0, 1, D_80091680[0]);
        func_80077C04(&pe->sp);
        if (func_80077CB4(pe, &pe->sp)) {
            func_800719E4(-1);
        }
        ps = &D_8009EC70[i].sp;
        func_80077B34(ps, 1);
        ps->u0 = 0x70;
        ps->v0 = D_80091680[-3];
        ps->w = 0x18;
        ps->h = 0xC;
        D_8009EC70[i].sp.clut = D_80091680[1];
        func_80077C84(pt = &D_8009ECA8[i], 0, 1, func_80077A64(0, 0, 0, 0) & 0xFFFF);
        func_80077C44(&pt->t);
        if (func_80077CB4(pt, &pt->t)) {
            func_800719E4(-1);
        }
        tl = &D_8009ECA8[i].t;
        tl->r0 = 2;
        tl->g0 = 2;
        tl->b0 = 2;
        tl->w = 0x140;
        tl->h = 0x36;
        tl->x0 = 0;
        tl->y0 = 0xAA;
        func_80077B04(tl, 1);
    }
}
