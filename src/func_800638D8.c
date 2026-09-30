typedef struct {
    unsigned addr : 24;
    unsigned len : 8;
} P_TAG;

#define setaddr(p, a) (((P_TAG *)(p))->addr = (unsigned int)(a))
#define getaddr(p) (((P_TAG *)(p))->addr)
#define addPrim(ot, p) setaddr(p, getaddr(ot)), setaddr(ot, p)

typedef struct Win {
    struct Win *next;
    int pad4;
    struct Menu *slot[4];
    unsigned char pad18[0x24];
    int f3C;
} Win;

typedef struct Menu {
    int pad0[14];
    int count;
    int w;
    int h;
    int cur_col;
    int cur_row;
    int pad4C[4];
    int base;
    int scroll;
    int flags;
    int pad68[6];
    int f80;
} Menu;

extern Win *D_8009D154;
extern Menu *D_8009D15C;
extern int D_8009D164;
extern int D_8009D168;
extern int D_8009D108;
extern unsigned char *D_8009D100;
extern unsigned char *D_8009D104;
extern unsigned int *D_8009D11C;
extern int *D_8009D12C;
extern int D_8009D124;
extern int D_8009D128;
extern int D_8009D0E8;
extern int D_800A22B0[];
extern int D_800A2270[];

extern void func_800527C0(int);
extern void func_80075B84(void *, short *);
extern void func_800634D4(Menu *, void (*)(int), int, int);
extern void func_8006374C(Menu *);
extern void func_80065260(int);
extern int func_8005E038(void);
extern void func_800622BC(int, int, int, int);

void func_800638D8(Menu *m, void (*draw)(int))
{
    Win *w;
    Win *found;
    int i;
    int sel;
    short r[4];
    short *rp;
    unsigned char *prim;
    unsigned char *q;
    register int *tt asm("$3");
    register int *rr asm("$5");
    int t0, t1;
    int hl;
    int k;
    int x;
    int f;
    int y;
    int b;
    int *r0;

    found = 0;
    w = D_8009D154;
    while (w != 0) {
        for (i = 0; i < 4; i++) {
            if (w->slot[i] == m) {
                break;
            }
        }
        if (i < 4) {
            found = w;
            break;
        }
        w = w->next;
    }
    sel = found->f3C;
    D_8009D164 = m->w;
    D_8009D168 = m->h;
    r[0] = 0;
    r[1] = 0;
    r[2] = 0x140;
    r[3] = 0xE0;
    if (D_8009D108 != 0) {
        r[1] = 0xE0;
    }
    rp = r;
    prim = 0;
    q = D_8009D100;
    if (q + 0xC < D_8009D104 + 0x4000) {
        D_8009D100 = q + 0xC;
        prim = q;
    } else {
        func_800527C0(1);
    }
    if (prim != 0) {
        func_80075B84(prim, rp);
    }
    addPrim(D_8009D11C, prim);
    r0 = D_8009D12C;
    if (r0 < D_800A22B0) {
        t0 = D_8009D124;
        t1 = D_8009D128;
        tt = r0 + 2;
        D_8009D12C = tt;
        r0[0] = t0;
        r0[1] = t1;
    } else {
        func_800527C0(2);
    }
    x = D_8009D124;
    y = m->scroll;
    D_8009D124 = x;
    D_8009D128 += y;
    if (m->scroll > 0) {
        D_8009D124 = x;
        D_8009D128 -= m->h;
        func_800634D4(m, draw, -1, sel);
    }
    for (k = 0; k < m->count; k++) {
        func_800634D4(m, draw, k, sel);
    }
    if (m->scroll < 0) {
        func_800634D4(m, draw, k, sel);
    }
    tt = D_8009D12C;
    if (D_800A2270 < tt) {
        t0 = tt[-2];
        t1 = tt[-1];
        tt -= 2;
        D_8009D12C = tt;
        D_8009D124 = t0;
        D_8009D128 = t1;
    } else {
        func_800527C0(3);
    }
    func_8006374C(m);
    func_80065260(m->f80);
    if (m->scroll != 0) {
        if (m->scroll > 0) {
            m->scroll -= m->h / 2;
            if (m->scroll < 0) {
                m->scroll = 0;
            }
        } else {
            m->scroll += m->h / 2;
            if (m->scroll > 0) {
                m->scroll = 0;
            }
        }
        if (m->scroll != 0) {
            return;
        }
    }
    if (m->cur_col < 0) {
        return;
    }
    if (m->cur_row < m->base) {
        return;
    }
    if (m->cur_row >= m->base + m->count) {
        return;
    }
    rr = D_8009D12C;
    if (rr < D_800A22B0) {
        t0 = D_8009D124;
        t1 = D_8009D128;
        tt = rr + 2;
        D_8009D12C = tt;
        rr[0] = t0;
        rr[1] = t1;
    } else {
        func_800527C0(2);
    }
    D_8009D124 += m->w * m->cur_col;
    D_8009D128 += m->h * (m->cur_row - m->base);
    hl = 0;
    if (!(m->flags & 0x80) && D_8009D15C == m) {
        f = 0;
        if (D_8009D0E8 != 0) {
            b = func_8005E038() & 0x20;
            f = b != 0;
        }
        if (f) {
            hl = 1;
        }
    }
    func_800622BC(m->w, m->h, hl, D_8009D15C == m);
    tt = D_8009D12C;
    if (D_800A2270 < tt) {
        t0 = tt[-2];
        t1 = tt[-1];
        tt -= 2;
        D_8009D12C = tt;
        D_8009D124 = t0;
        D_8009D128 = t1;
    } else {
        func_800527C0(3);
    }
}
