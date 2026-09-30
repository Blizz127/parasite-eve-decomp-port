typedef struct {
    short vx, vy, vz, pad;
} SVECTOR;

typedef struct {
    long vx, vy, vz, pad;
} VECTOR;

typedef struct {
    short m[3][3];
    long t[3];
} MATRIX;

typedef struct {
    unsigned int tag;
    unsigned char r0, g0, b0, code;
    short x0, y0;
    short x1, y1;
} LINE_F2;

typedef struct {
    unsigned int tag;
    unsigned char r0, g0, b0, code;
    short x0, y0;
    short x1, y1;
    short x2, y2;
} LINE_F3;

typedef struct {
    unsigned int tag;
    unsigned char r0, g0, b0, code;
    short x0, y0;
    short x1, y1;
    short x2, y2;
} POLY_F3;

typedef struct {
    unsigned int tp[2];
    LINE_F3 l;
} LEnt; /* 28 bytes */

/* the same 28-byte record viewed from its prim: retail keeps &D_8009E460[0].l as the
   indexed base register and addresses the prim fields with 4..10 displacements */
typedef struct {
    LINE_F3 l;
    unsigned int tp[2];
} LRec;

typedef struct {
    LINE_F2 a;
    LINE_F2 b;
} LinePair; /* 32 bytes */

typedef struct {
    short x, y;
    unsigned char pad4[44];
} Rec48;

typedef struct {
    unsigned char pad0[100];
    short f64;
    short f66;
} Ent;

extern int D_8009CDDC;
extern int D_8009CDDC_b asm("D_8009CDDC");
extern int D_8009D250;
extern LEnt D_8009E460[];
extern LinePair D_8009E498[];
extern POLY_F3 D_8009E4D8[];
extern Rec48 D_8009E360[];
extern unsigned char *D_800B0E38[];

extern void func_800794C4(SVECTOR *r, MATRIX *m);
extern void func_80078E04(MATRIX *m);
extern int *func_80078C94(int *m, int *v);
extern void func_80078E94(MATRIX *m);
extern void func_800792D4(SVECTOR *v0, VECTOR *v1, long *flag);
extern void func_80077AC4(void *a0, void *a1);

void func_80031760(Ent *e, signed char flag) {
    MATRIX m1;
    MATRIX m2;
    SVECTOR tri[3] = { { -3, -16, 0, 0 }, { 3, -16, 0, 0 }, { 0, 0, 0, 0 } };
    VECTOR out[3];
    SVECTOR r = { 0, 0, (D_8009D250 << 6) & 0xFFF };
    VECTOR trans = { e->f64, e->f66, 0 };
    long flg;
    LRec *base;
    POLY_F3 *p;
    POLY_F3 *pb;
    short y;
    short z;

    base = (LRec *)&D_8009E460[0].l;
    base[D_8009CDDC].l.x0 = e->f64 - 12;
    base[D_8009CDDC].l.y0 = e->f66 - 12;
    if (flag == 0) {
        base[D_8009CDDC].l.r0 = 150;
        base[D_8009CDDC].l.g0 = 20;
        base[D_8009CDDC].l.b0 = 20;
        D_8009E4D8[D_8009CDDC].r0 = 150;
        D_8009E4D8[D_8009CDDC].g0 = 20;
        D_8009E4D8[D_8009CDDC].b0 = 20;
    } else {
        base[D_8009CDDC].l.r0 = 50;
        base[D_8009CDDC].l.g0 = 10;
        base[D_8009CDDC].l.b0 = 10;
        D_8009E4D8[D_8009CDDC].r0 = 50;
        D_8009E4D8[D_8009CDDC].g0 = 10;
        D_8009E4D8[D_8009CDDC_b].b0 = 10;
    }
    func_800794C4(&r, &m1);
    func_80078E04(&m1);
    func_80078C94((int *)&m2, (int *)&trans);
    func_80078E94(&m2);
    func_800792D4(&tri[0], &out[0], &flg);
    func_800792D4(&tri[1], &out[1], &flg);
    func_800792D4(&tri[2], &out[2], &flg);
    {
        int pad[8]; /* 32 unexplained frame bytes: retail frame 0x120, live locals end at 0xCC */
    }
    pb = D_8009E4D8;
    p = &pb[D_8009CDDC];
    p->x0 = out[0].vx;
    p->y0 = out[0].vy;
    p->x1 = out[1].vx;
    p->y1 = out[1].vy;
    p->x2 = out[2].vx;
    p->y2 = out[2].vy;
    func_80077AC4(D_800B0E38[D_8009CDDC] + 20, p);
    y = e->f64;
    if (y + 134 >= 301) {
        short x;

        y -= 8;
        D_8009E498[D_8009CDDC].a.x0 = D_8009E498[D_8009CDDC].b.x0 = y;
        x = e->f64;
        D_8009E498[D_8009CDDC].a.x1 = D_8009E498[D_8009CDDC].b.x1 = x - 35;
        D_8009E360[D_8009CDDC].x = x - 115;
    } else {
        short x;
        register short t asm("$2");

        t = y + 8;
        D_8009E498[D_8009CDDC].a.x0 = D_8009E498[D_8009CDDC].b.x0 = t;
        x = e->f64 + 35;
        D_8009E360[D_8009CDDC].x = x;
        D_8009E498[D_8009CDDC].a.x1 = D_8009E498[D_8009CDDC].b.x1 = x;
    }
    z = e->f66;
    if (z - 82 < 0) {
        short x;
        short t;

        t = z + 8;
        D_8009E498[D_8009CDDC].a.y0 = t;
        D_8009E498[D_8009CDDC].b.y0 = e->f66 + 9;
        x = e->f66 + 35;
        D_8009E360[D_8009CDDC].y = x;
        D_8009E498[D_8009CDDC].a.y1 = x;
        D_8009E498[D_8009CDDC].b.y1 = e->f66 + 36;
    } else {
        short x;
        short t;

        t = z - 8;
        D_8009E498[D_8009CDDC].a.y0 = t;
        D_8009E498[D_8009CDDC].b.y0 = e->f66 - 7;
        x = e->f66 - 35;
        D_8009E360[D_8009CDDC].y = x;
        D_8009E498[D_8009CDDC].a.y1 = x;
        D_8009E498[D_8009CDDC].b.y1 = e->f66 - 34;
    }
    func_80077AC4(D_800B0E38[D_8009CDDC] + 28, &D_8009E498[D_8009CDDC].b);
    func_80077AC4(D_800B0E38[D_8009CDDC] + 24, &D_8009E498[D_8009CDDC].a);
    func_80077AC4(D_800B0E38[D_8009CDDC] + 20, &D_8009E460[D_8009CDDC]);
}
