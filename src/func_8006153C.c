typedef struct Tag {
    unsigned int addr : 24;
    unsigned int len : 8;
} Tag;

typedef struct {
    short y;
    short x;
} Pt;

typedef struct PolyF3 {
    unsigned int tag;
    unsigned char r0, g0, b0, code;
    short x0, y0;
    short x1, y1;
    short x2, y2;
} PolyF3;

extern unsigned int *D_8009D100;
extern unsigned int D_8009D104;
extern int D_8009D10C;
extern unsigned int D_8009D110;
extern unsigned int D_8009D114;
extern unsigned int *D_8009D11C;
extern int D_8009D14C[];
extern Pt D_800A22B0[];
extern void func_800527C0(int);
extern int abs(int);

void func_8006153C(int i0, int i1, int d, int sel)
{
    PolyF3 *p;
    PolyF3 *q;
    PolyF3 *r;
    unsigned int *cur;
    unsigned int base;
    Pt a0;
    Pt b0;
    Pt a;
    Pt b;
    int ad;
    int t;
    int t2;
    int c;

    cur = D_8009D100;
    base = D_8009D104;
    p = 0;
    ad = abs(d);
    if (cur + 10 < (unsigned int *)(base + 0x4000)) {
        D_8009D100 = cur + 10;
        p = (PolyF3 *)cur;
    } else {
        func_800527C0(1);
    }
    if (p != 0) {
        for (r = p; r < p + 2; r++) {
            if (D_8009D10C != 0) {
                *(unsigned int *)&r->r0 = D_8009D114;
            } else {
                *(unsigned int *)&r->r0 = D_8009D110;
            }
            ((unsigned char *)r)[3] = 4;
            r->code = 0x20;
        }
    }
    a = D_800A22B0[i0 & 0x7F];
    a0 = a;
    b = D_800A22B0[i1 & 0x7F];
    b0 = b;
    q = p;
    q[0].code |= 2;
    q[1].code |= 2;
    c = D_8009D14C[sel];
    if (a0.y == b0.y) {
        a.x += ad;
        t = b.x;
        b.x = (i1 & 0x80) ? t + ad : t - ad;
        a.y += d;
        b.y += d;
        *(unsigned int *)&q[0].r0 = (*(unsigned int *)&q[0].r0 & 0xFF000000) | c;
        *(unsigned int *)&q[1].r0 = (*(unsigned int *)&q[1].r0 & 0xFF000000) | c;
    } else {
        a.x += d;
        b.x += d;
        a.y += ad;
        t2 = b.y;
        b.y = (i1 & 0x80) ? t2 + ad : t2 - ad;
        *(unsigned int *)&q[0].r0 = (*(unsigned int *)&q[0].r0 & 0xFF000000) | ((c >> 1) & 0x7F7F7F);
        *(unsigned int *)&q[1].r0 = (*(unsigned int *)&q[1].r0 & 0xFF000000) | ((c >> 1) & 0x7F7F7F);
    }
    q[0].x0 = a0.x;
    q[0].x1 = q[1].x2 = b0.x;
    q[0].x2 = q[1].x1 = a.x;
    q[0].y0 = a0.y;
    q[0].y1 = q[1].y2 = b0.y;
    q[0].y2 = q[1].y1 = a.y;
    q[1].x0 = b.x;
    q[1].y0 = b.y;
    ((Tag *)q)->addr = ((Tag *)D_8009D11C)->addr;
    ((Tag *)D_8009D11C)->addr = (unsigned int)q;
    ((Tag *)(q + 1))->addr = ((Tag *)D_8009D11C)->addr;
    ((Tag *)D_8009D11C)->addr = (unsigned int)(q + 1);
}
