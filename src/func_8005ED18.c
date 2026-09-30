typedef struct Tag {
    unsigned int addr : 24;
    unsigned int len : 8;
} Tag;

typedef struct PolyFT4 {
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
} PolyFT4;

extern unsigned int *D_8009D100;
extern unsigned int D_8009D104;
extern int D_8009D10C;
extern unsigned int D_8009D110;
extern unsigned int D_8009D114;
extern unsigned int *D_8009D11C;
extern int D_8009D124;
extern int D_8009D128;
extern void func_800527C0(int);
extern unsigned char *func_8005DADC(int);

void func_8005ED18(int id, int mode)
{
    PolyFT4 *p;
    register unsigned char *g asm("$16");

    p = 0;
    g = func_8005DADC(id);
    if (D_8009D100 + 10 < (unsigned int *)(D_8009D104 + 0x4000)) {
        D_8009D100 = D_8009D100 + 10;
        p = (PolyFT4 *)(D_8009D100 - 10);
    } else {
        func_800527C0(1);
    }
    if (p != 0) {
        if (D_8009D10C != 0) {
            *(unsigned int *)&p->r0 = D_8009D114;
        } else {
            *(unsigned int *)&p->r0 = D_8009D110;
        }
        ((unsigned char *)p)[3] = 9;
        p->code = 0x2C;
    }
    {
        register PolyFT4 *q asm("$4") = p;
        q->x0 = q->x2 = D_8009D124;
        q->y0 = q->y1 = D_8009D128;
        q->x1 = q->x3 = q->x0 + g[4];
        q->y2 = q->y3 = q->y0 + g[5];
        if (mode == 2) {
            q->u0 = q->u2 = g[0] + g[4] - 1;
            q->v0 = q->v1 = g[1] + g[5] - 1;
            q->u1 = q->u3 = g[0] - 1;
            q->v2 = q->v3 = g[1] - 1;
        }
    }
    p->clut = *(unsigned short *)(g + 2);
    p->tpage = 7;
    ((Tag *)p)->addr = ((Tag *)D_8009D11C)->addr;
    ((Tag *)D_8009D11C)->addr = (unsigned int)p;
}
