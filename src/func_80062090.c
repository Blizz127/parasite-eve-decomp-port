typedef struct Tag {
    unsigned int addr : 24;
    unsigned int len : 8;
} Tag;

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
extern int D_8009D124;
extern int D_8009D128;
extern void func_800527C0(int);
extern int func_80073A44(int);
extern void func_80077C84(void *a0, int a1, int a2, int a3);

void func_80062090(int w, int h, int blink)
{
    Tile *p;
    void *q;
    int v;
    int c;

    p = 0;
    if (D_8009D100 + 4 < (unsigned int *)(D_8009D104 + 0x4000)) {
        D_8009D100 = D_8009D100 + 4;
        p = (Tile *)(D_8009D100 - 4);
    } else {
        func_800527C0(1);
    }
    if (p != 0) {
        if (D_8009D10C != 0) {
            *(unsigned int *)&p->r0 = D_8009D114;
        } else {
            *(unsigned int *)&p->r0 = D_8009D110;
        }
        ((unsigned char *)p)[3] = 3;
        p->code = 0x60;
    }
    if (blink) {
        v = func_80073A44(-1);
        if (v & 0x20) {
            p->b0 = (v & 0x1F) << 1;
        } else {
            p->b0 = 0x40 - ((v & 0x1F) << 1);
        }
        p->r0 = p->g0 = p->b0 = p->b0;
    } else {
        *(unsigned int *)&p->r0 &= 0xFF000000;
        *(unsigned int *)&p->r0 |= D_8009D114 & 0xFFFFFF;
    }
    p->w = w - 4;
    p->h = h - 4;
    p->code |= 2;
    p->x0 = D_8009D124 + 2;
    p->y0 = D_8009D128 + 2;
    ((Tag *)p)->addr = ((Tag *)D_8009D11C)->addr;
    ((Tag *)D_8009D11C)->addr = (unsigned int)p;
    q = 0;
    if (D_8009D100 + 2 < (unsigned int *)(D_8009D104 + 0x4000)) {
        D_8009D100 = D_8009D100 + 2;
        q = D_8009D100 - 2;
    } else {
        func_800527C0(1);
    }
    if (q != 0) {
        func_80077C84(q, 0, 0, 0x20);
    }
    ((Tag *)q)->addr = ((Tag *)D_8009D11C)->addr;
    ((Tag *)D_8009D11C)->addr = (unsigned int)q;
}
