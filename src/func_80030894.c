typedef struct {
    unsigned int tag;
    unsigned int code[1];
} DR_TPAGE;

typedef struct {
    unsigned int tag;
    unsigned char r0, g0, b0, code;
    short x0, y0;
    unsigned char u0, v0;
    unsigned short clut;
    short w, h;
} SPRT;

typedef struct {
    DR_TPAGE tp;
    SPRT s;
} Spr;

typedef struct {
    unsigned int tag;
    unsigned char r0, g0, b0, code;
    short x0, y0;
    short w, h;
} TILE;

typedef struct {
    DR_TPAGE tp;
    TILE t;
} TileT;

typedef struct {
    unsigned int tag;
    unsigned char r0, g0, b0, code;
    short x0, y0;
    unsigned char r1, g1, b1, pad1;
    short x1, y1;
    unsigned char r2, g2, b2, pad2;
    short x2, y2;
    unsigned char r3, g3, b3, pad3;
    short x3, y3;
} POLY_G4;

typedef struct {
    unsigned int tag;
    unsigned char r0, g0, b0, code;
    short x0, y0;
    unsigned char u0, v0;
    unsigned short clut;
    short x1, y1;
    unsigned char u1, v1;
    unsigned short tpage;
    short x2, y2;
    unsigned char u2, v2;
    unsigned short pad1;
    short x3, y3;
    unsigned char u3, v3;
    unsigned short pad2;
} POLY_FT4;

typedef struct {
    unsigned int tag;
    unsigned char r0, g0, b0, code;
    short x0, y0;
    short x1, y1;
    short x2, y2;
} POLY_F3;

typedef struct {
    unsigned char c[3];
} Col3;

extern Col3 D_8009CD90;
extern POLY_FT4 D_800BE9F0[2];
extern Spr D_800B01C0[2][10][5];
extern TileT D_8009E068[2];
extern TILE D_8009E098[2];
extern POLY_G4 D_800B00E8[2];
extern Spr D_800B6920[2];
extern Spr D_8009E0F0[2][4];
extern Spr D_8009E1D0[2][5];
extern POLY_G4 D_800B0130[2][2];
extern Spr D_8009E0B8[2];
extern Spr D_8009E2E8[2];
extern Spr D_8009E320[2];
extern TILE D_8009E358[2][3];
extern Spr D_8009E460[2];
extern TILE D_8009E498[2][2];
extern POLY_F3 D_8009E4D8[2];
extern Spr D_8009E3B8[2][3];
extern Spr D_8009E500[2][10];
extern Spr D_8009E768[2];
extern Spr D_8009E7A0[2][4];
extern Spr D_8009E730[2];
extern Spr D_8009E880[2];
extern Spr D_8009E8B8[2][2];
extern Spr D_8009E928[2];
extern Spr D_8009E960[2][13];
extern Spr D_8009EC38[2];

extern int func_80077A64(int, int, int, int);
extern int func_80077AA4(int, int);
extern unsigned char *func_8005DADC(int);
extern void func_80077BA4(void *);
extern void func_80077B04(void *, int);
extern void func_80077B34(void *, int);
extern void func_80077BC4(void *);
extern void func_80077C44(void *);
extern void func_80077C64(void *);
extern void func_80077B64(void *);
extern void func_800370DC(Spr *, int);
extern void func_80037140(TileT *, unsigned short);

void func_80030894(void)
{
    Col3 col;
    int tp;
    unsigned char buf;
    unsigned short clut;
    unsigned char *p;
    unsigned char i;
    unsigned char j;

    col = D_8009CD90;
    tp = func_80077A64(0, 1, 256, 480) & 0xFFFF;
    clut = func_80077AA4(304, 504);
    buf = 0;
    do {
        p = func_8005DADC(139);
        {
            POLY_FT4 *f = &D_800BE9F0[buf];

            func_80077BA4(f);
            f->u0 = p[0];
            f->v0 = p[1];
            f->u1 = p[0] + p[4];
            f->v1 = p[1];
            f->u2 = p[0];
            f->v2 = p[1] + p[5];
            f->u3 = p[0] + p[4];
            f->v3 = p[1] + p[5];
            D_800BE9F0[buf].tpage = func_80077A64(0, 0, 448, 0);
            D_800BE9F0[buf].clut = *(unsigned short *)(p + 2);
            f->x0 = 0;
            f->y0 = 0;
            f->x1 = 0;
            f->y1 = 0;
            f->x2 = 0;
            f->y2 = 0;
            f->x3 = 0;
            f->y3 = 0;
            f->r0 = 0;
            f->g0 = 0;
            f->b0 = 0;
            func_80077B04(f, 1);
        }

        for (i = 0; i < 10; i++) {
            for (j = 0; j < 4; j++) {
                func_800370DC(&D_800B01C0[buf][i][j], tp);
                D_800B01C0[buf][i][j].s.clut = clut;
            }
        }

        func_80037140(&D_8009E068[buf], func_80077A64(0, 0, 0, 0) & 0xFFFF);
        {
            TILE *t = (TILE *)((char *)&D_8009E068[0].t + buf * sizeof(TileT));

            t->r0 = 48;
            t->g0 = 48;
            t->b0 = 48;
            func_80077B04(t, 1);
        }
        {
            TILE *t = &D_8009E098[buf];
            POLY_G4 *g;
            SPRT *s;

            func_80077C44(t);
            g = &D_800B00E8[buf];
            t->r0 = 29;
            t->g0 = 62;
            t->b0 = 50;
            t->w = 56;
            t->h = 3;
            func_80077BC4(g);
            func_800370DC(&D_800B6920[buf], tp);
            s = (SPRT *)((char *)&D_800B6920[0].s + buf * sizeof(Spr));
            s->u0 = 200;
            s->v0 = 224;
            D_800B6920[buf].s.clut = clut;
            s->w = 4;
            s->h = 8;
            g->r0 = 0;
            g->g0 = 70;
            g->b0 = 130;
            g->r1 = 159;
            g->g1 = 255;
            g->b1 = 249;
            g->r2 = 0;
            g->g2 = 70;
            g->b2 = 130;
            g->r3 = 159;
            g->g3 = 255;
            g->b3 = 249;
            s->r0 = 159;
            s->g0 = 255;
            s->b0 = 249;
        }

        for (i = 0; i < 4; i++) {
            SPRT *s;

            func_800370DC(&D_8009E0F0[buf][i], tp);
            D_8009E0F0[buf][i].s.clut = clut;
            s = (SPRT *)((char *)&D_8009E0F0[0][0].s + (i * sizeof(Spr) + buf * sizeof(D_8009E0F0[0])));
            s->w = 6;
            s->h = 10;
        }
        for (i = 0; i < 5; i++) {
            SPRT *s;

            func_800370DC(&D_8009E1D0[buf][i], tp);
            D_8009E1D0[buf][i].s.clut = clut;
            s = (SPRT *)((char *)&D_8009E1D0[0][0].s + (i * sizeof(Spr) + buf * sizeof(D_8009E1D0[0])));
            s->w = 6;
            s->h = 10;
        }

        {
            POLY_G4 *g0 = &D_800B0130[buf][0];
            POLY_G4 *g1;

            func_80077BC4(g0);
            g1 = (POLY_G4 *)((char *)&D_800B0130[0][1] + buf * sizeof(D_800B0130[0]));
            func_80077BC4(g1);
            g0->r0 = 0;
            g0->g0 = 130;
            g0->b0 = 54;
            g0->r1 = 74;
            g0->g1 = 255;
            g0->b1 = 59;
            g0->r2 = 0;
            g0->g2 = 130;
            g0->b2 = 54;
            g0->r3 = 74;
            g0->g3 = 255;
            g0->b3 = 59;
            g1->r0 = 255;
            g1->g0 = 61;
            g1->b0 = 129;
            g1->r1 = 131;
            g1->g1 = 19;
            g1->b1 = 1;
            g1->r2 = 255;
            g1->g2 = 61;
            g1->b2 = 129;
            g1->r3 = 131;
            g1->g3 = 19;
            g1->b3 = 1;
        }
        {
            SPRT *s;

            func_800370DC(&D_8009E0B8[buf], tp);
            s = (SPRT *)((char *)&D_8009E0B8[0].s + buf * sizeof(Spr));
            func_80077B34(s, 1);
            s->u0 = 80;
            s->v0 = 244;
            s->r0 = 128;
            s->g0 = 128;
            s->b0 = 128;
            D_8009E0B8[buf].s.clut = clut;
            s->w = 8;
            s->h = 4;
        }
        {
            SPRT *s;

            func_800370DC(&D_8009E2E8[buf], tp);
            s = (SPRT *)((char *)&D_8009E2E8[0].s + buf * sizeof(Spr));
            func_80077B34(s, 1);
            s->u0 = 88;
            s->v0 = 244;
            s->r0 = 128;
            s->g0 = 128;
            s->b0 = 128;
            D_8009E2E8[buf].s.clut = clut;
            s->w = 8;
            s->h = 4;
        }
        {
            SPRT *s;

            func_800370DC(&D_8009E320[buf], tp);
            s = (SPRT *)((char *)&D_8009E320[0].s + buf * sizeof(Spr));
            func_80077B34(s, 1);
            s->u0 = 96;
            s->v0 = 244;
            s->r0 = 128;
            s->g0 = 128;
            s->b0 = 128;
            D_8009E320[buf].s.clut = clut;
            s->w = 8;
            s->h = 4;
        }

        for (i = 0; i < 3; i++) {
            TILE *t;

            func_80077C44(&D_8009E358[buf][i]);
            t = (TILE *)((char *)&D_8009E358[0][0] + (buf * sizeof(D_8009E358[0]) + i * sizeof(TILE)));
            {
                unsigned char c = col.c[i];

                t->r0 = c;
                t->g0 = c;
                t->b0 = c;
            }
        }

        {
            SPRT *s;

            func_800370DC(&D_8009E460[buf], tp);
            s = (SPRT *)((char *)&D_8009E460[0].s + buf * sizeof(Spr));
            s->u0 = 232;
            s->v0 = 224;
            D_8009E460[buf].s.clut = clut;
            s->w = 24;
            s->h = 24;
        }
        {
            TILE *t0 = &D_8009E498[buf][0];
            TILE *t1;

            func_80077C64(t0);
            t1 = (TILE *)((char *)&D_8009E498[0][1] + buf * sizeof(D_8009E498[0]));
            func_80077C64(t1);
            t0->r0 = 224;
            t0->g0 = 224;
            t0->b0 = 224;
            t1->r0 = 96;
            t1->g0 = 96;
            t1->b0 = 96;
            func_80077B64(&D_8009E4D8[buf]);
        }

        for (i = 0; i < 3; i++) {
            SPRT *s;

            func_800370DC(&D_8009E3B8[buf][i], tp);
            D_8009E3B8[buf][i].s.clut = clut;
            s = (SPRT *)((char *)&D_8009E3B8[0][0].s + (i * sizeof(Spr) + buf * sizeof(D_8009E3B8[0])));
            s->w = 24;
            s->h = 8;
            s->r0 = 128;
            s->g0 = 128;
            s->b0 = 128;
        }
        for (i = 0; i < 10; i++) {
            SPRT *s;

            func_800370DC(&D_8009E500[buf][i], tp);
            s = (SPRT *)((char *)&D_8009E500[0][0].s + (buf * sizeof(D_8009E500[0]) + i * sizeof(Spr)));
            s->r0 = 128;
            s->g0 = 128;
            s->b0 = 128;
        }

        {
            SPRT *s;

            func_800370DC(&D_8009E768[buf], tp);
            s = (SPRT *)((char *)&D_8009E768[0].s + buf * sizeof(Spr));
            s->u0 = 88;
            s->v0 = 239;
            D_8009E768[buf].s.clut = clut;
            s->w = 36;
            s->h = 5;
            s->r0 = 128;
            s->g0 = 128;
            s->b0 = 128;
        }
        for (i = 0; i < 4; i++) {
            SPRT *s;

            func_800370DC(&D_8009E7A0[buf][i], tp);
            D_8009E7A0[buf][i].s.clut = clut;
            s = (SPRT *)((char *)&D_8009E7A0[0][0].s + (i * sizeof(Spr) + buf * sizeof(D_8009E7A0[0])));
            s->w = 6;
            s->h = 6;
            s->r0 = 128;
            s->g0 = 128;
            s->b0 = 128;
        }

        {
            SPRT *s;

            func_800370DC(&D_8009E730[buf], tp);
            s = (SPRT *)((char *)&D_8009E730[0].s + buf * sizeof(Spr));
            s->u0 = 104;
            s->v0 = 244;
            D_8009E730[buf].s.clut = func_80077AA4(304, 505);
            s->w = 24;
            s->h = 4;
            s->r0 = 128;
            s->g0 = 128;
            s->b0 = 128;
        }
        {
            SPRT *s;

            func_800370DC(&D_8009E880[buf], tp);
            s = (SPRT *)((char *)&D_8009E880[0].s + buf * sizeof(Spr));
            s->u0 = 124;
            s->v0 = 239;
            D_8009E880[buf].s.clut = clut;
            s->w = 36;
            s->h = 5;
            s->r0 = 128;
            s->g0 = 128;
            s->b0 = 128;
        }
        for (i = 0; i < 2; i++) {
            SPRT *s;

            func_800370DC(&D_8009E8B8[buf][i], tp);
            D_8009E8B8[buf][i].s.clut = clut;
            s = (SPRT *)((char *)&D_8009E8B8[0][0].s + (i * sizeof(Spr) + buf * sizeof(D_8009E8B8[0])));
            s->w = 6;
            s->h = 6;
        }

        {
            SPRT *s;

            func_800370DC(&D_8009E928[buf], tp);
            D_8009E928[buf].s.clut = clut;
            s = (SPRT *)((char *)&D_8009E928[0].s + buf * sizeof(Spr));
            s->r0 = 128;
            s->g0 = 128;
            s->b0 = 0;
        }
        for (i = 0; i < 13; i++) {
            SPRT *s;

            p = func_8005DADC(i + 106);
            func_800370DC(&D_8009E960[buf][i], func_80077A64(0, 0, 448, 0) & 0xFFFF);
            s = (SPRT *)((char *)&D_8009E960[0][0].s + (buf * sizeof(D_8009E960[0]) + i * sizeof(Spr)));
            s->u0 = p[0];
            s->v0 = p[1];
            D_8009E960[buf][i].s.clut = *(unsigned short *)(p + 2);
            s->w = p[4];
            s->h = p[5];
            s->r0 = 128;
            s->g0 = 128;
            s->b0 = 128;
        }

        {
            SPRT *s;

            func_800370DC(&D_8009EC38[buf], func_80077A64(0, 0, 448, 0) & 0xFFFF);
            s = (SPRT *)((char *)&D_8009EC38[0].s + buf * sizeof(Spr));
            s->w = 16;
            s->h = 16;
            s->r0 = 128;
            s->g0 = 128;
            s->b0 = 128;
        }
        buf++;
    } while (buf < 2);
}
