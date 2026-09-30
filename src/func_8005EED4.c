/*
 * func_8005EED4 -- font glyph emitter: FT4 packet alloc, glyph metrics via
 * func_8005DC28, UV/XY fill, tpage, link into the D_8009D11C OT slot and
 * advance the pen.  Era -O2 -G8 (era_o2_g8).
 *
 * Load-bearing levers (lane exeF): `short x, y` gives the x/y adds their own
 * early truncation insns (sched tie x < y < u1 by LUID); `unsigned char u, v`
 * removes the separate QI truncation insns; the empty volatile
 * `asm("" : : "r"(v))` after the v computation is a sched barrier that keeps
 * the u chain ahead of the v chain and raises v's local-alloc priority.
 */
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

extern int D_8009D0D8;
extern int D_8009CDB0;
extern void func_8005EB64(int);
extern int func_8005DC28(int);
extern int func_80077A64(int, int, int, int);

static inline void move_pen(int dx, int dy)
{
    D_8009D124 += dx;
    D_8009D128 += dy;
}

void func_8005EED4(unsigned int c)
{
    int code;
    int n;
    int row;
    int col;
    int w;
    int wd;
    int f;
    int lo;
    unsigned char u;
    unsigned char v;
    short x;
    short y;
    PolyFT4 *p;

    code = c & 0xFF;
    if (D_8009D0D8 != 0) {
        code += D_8009D0D8 << 8;
        D_8009D0D8 = 0;
    }
    if ((c & 0xFF) >= 0xFA) {
        D_8009D0D8 = (c & 0xFF) - 0xFA;
        code = -1;
    }
    n = code;
    if (n == 0x100) {
        func_8005EB64(0x77);
        move_pen(12, 0);
        return;
    }
    if (n < 0) {
        return;
    }
    if (n > 0x100) {
        n -= 0x13;
    }
    n %= 441;
    row = n / 21;
    col = n - row * 21;
    p = 0;
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
    w = func_8005DC28(n);
    lo = w & 0xF;
    wd = (w >> 4) & 0xF;
    f = 0;
    if (n < 10 || n == 15) {
        f = 1;
    }
    D_8009CDB0 = f + 1;
    u = col * 12 + lo;
    v = row * 12;
    asm("" : : "r"(v));
    x = D_8009D124 + (D_8009CDB0 >> 1);
    y = D_8009D128 + 1;
    p->u0 = p->u2 = u;
    p->clut = 0x89C;
    p->v0 = p->v1 = v;
    p->x0 = p->x2 = x;
    p->u1 = p->u3 = p->u0 + wd;
    p->y0 = p->y1 = y;
    p->v2 = p->v3 = p->v0 + 12;
    p->x1 = p->x3 = p->x0 + wd;
    p->y2 = p->y3 = p->y0 + 12;
    p->tpage = func_80077A64(0, 0, 0x140, 0);
    ((Tag *)p)->addr = ((Tag *)D_8009D11C)->addr;
    ((Tag *)D_8009D11C)->addr = (unsigned int)p;
    move_pen(wd + D_8009CDB0, 0);
}
