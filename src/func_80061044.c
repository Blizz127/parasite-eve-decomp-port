typedef struct Tag {
    unsigned int addr : 24;
    unsigned int len : 8;
} Tag;

typedef struct PolyG4 {
    unsigned int tag;
    unsigned char r0, g0, b0, code;
    short x0, y0;
    unsigned char r1, g1, b1, pad1;
    short x1, y1;
    unsigned char r2, g2, b2, pad2;
    short x2, y2;
    unsigned char r3, g3, b3, pad3;
    short x3, y3;
} PolyG4;

typedef struct Tile {
    unsigned int tag;
    unsigned char r0, g0, b0, code;
    short x0, y0;
    short w, h;
} Tile;

extern unsigned int *D_8009D100;
extern unsigned int D_8009D104;
extern int D_8009D10C;
extern unsigned int D_8009D110;
extern unsigned int D_8009D114;
extern unsigned int *D_8009D11C;
extern void func_800527C0(int);
extern int func_8005257C(void);
extern void func_80077C84(void *a0, int a1, int a2, int a3);

void func_80061044(int a0, int a1)
{
    PolyG4 *h;
    PolyG4 *p;
    Tile *t;
    void *m;
    int dy;
    int w;
    short rect[4];

    w = 0;
    dy = (func_8005257C() == 0) ? 7 : 0;
    if (D_8009D100 + 9 < (unsigned int *)(D_8009D104 + 0x4000)) {
        D_8009D100 = D_8009D100 + 9;
        w = (int)(D_8009D100 - 9);
    } else {
        func_800527C0(1);
    }
    if (w != 0) {
        *(unsigned int *)&((PolyG4 *)w)->r0 = 0x808080;
        ((unsigned char *)w)[3] = 8;
        ((PolyG4 *)w)->code = 0x38;
    }
    p = (PolyG4 *)w;
    if (a1 > 0 && ((a1 < a0) ? a1 : a0) >= 0) {
        w = (((a1 < a0) ? a1 : a0) * 56) / a1;
    } else {
        w = 0;
    }
    p->g0 = p->g2 = 130;
    p->b0 = p->b2 = 54;
    p->r1 = p->r3 = 74;
    p->g1 = p->g3 = 255;
    p->b1 = p->b3 = 59;
    p->r0 = p->r2 = 0;
    p->y0 = p->y1 = dy + 170;
    p->x1 = p->x3 = w + 229;
    p->x0 = p->x2 = 229;
    p->y2 = p->y3 = dy + 173;
    ((Tag *)p)->addr = ((Tag *)D_8009D11C)->addr;
    ((Tag *)D_8009D11C)->addr = (unsigned int)p;
    h = 0;
    if (D_8009D100 + 9 < (unsigned int *)(D_8009D104 + 0x4000)) {
        D_8009D100 = D_8009D100 + 9;
        h = (PolyG4 *)(D_8009D100 - 9);
    } else {
        func_800527C0(1);
    }
    if (h != 0) {
        *(unsigned int *)&h->r0 = 0x808080;
        ((unsigned char *)h)[3] = 8;
        h->code = 0x38;
    }
    p = h;
    p->r0 = p->r2 = 255;
    p->g0 = p->g2 = 61;
    p->b0 = p->b2 = 129;
    p->r1 = p->r3 = 131;
    p->g1 = p->g3 = 19;
    p->b1 = p->b3 = 1;
    p->x0 = p->x2 = w + 229;
    p->y0 = p->y1 = dy + 170;
    p->x1 = p->x3 = 285;
    p->y2 = p->y3 = dy + 173;
    ((Tag *)p)->addr = ((Tag *)D_8009D11C)->addr;
    D_8009D110 = 0x303030;
    D_8009D114 = 0x181818;
    D_8009D10C = 0;
    ((Tag *)D_8009D11C)->addr = (unsigned int)p;
    t = 0;
    if (D_8009D100 + 4 < (unsigned int *)(D_8009D104 + 0x4000)) {
        D_8009D100 = D_8009D100 + 4;
        t = (Tile *)(D_8009D100 - 4);
    } else {
        func_800527C0(1);
    }
    if (t != 0) {
        if (D_8009D10C != 0) {
            *(unsigned int *)&t->r0 = D_8009D114;
        } else {
            *(unsigned int *)&t->r0 = D_8009D110;
        }
        ((unsigned char *)t)[3] = 3;
        t->code = 0x60;
    }
    t->x0 = 221;
    t->y0 = dy + 166;
    t->w = 84;
    t->h = 11;
    t->code |= 2;
    ((Tag *)t)->addr = ((Tag *)D_8009D11C)->addr;
    ((Tag *)D_8009D11C)->addr = (unsigned int)t;
    m = 0;
    if (D_8009D100 + 2 < (unsigned int *)(D_8009D104 + 0x4000)) {
        D_8009D100 = D_8009D100 + 2;
        m = D_8009D100 - 2;
    } else {
        func_800527C0(1);
    }
    if (m != 0) {
        func_80077C84(m, 0, 0, 0);
    }
    ((Tag *)m)->addr = ((Tag *)D_8009D11C)->addr;
    D_8009D110 = 0x808080;
    D_8009D114 = 0x404040;
    ((Tag *)D_8009D11C)->addr = (unsigned int)m;
}
