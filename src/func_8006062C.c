/*
 * func_8006062C — segmented gauge draw (VRAM 0x8006062C, file 0x50E2C, 0xA18).
 * Sprite record from func_8005DADC(0x48); pen pushed twice on the D_8009D12C
 * stack, label via func_8005FA3C, then up to six POLY_FT4 segments (left cap,
 * stretch, right caps selected by n) from the D_8009D100 pool, pen popped,
 * DR_MODE with the record's tpage bits. era gcc-2.7.2 -O2 -G8.
 * The store pointer p is one function-scope variable copied from each
 * segment's pool pointer g after its init (retail keeps it in $a3); the
 * right-cap x0 must be spelled D_8009D124 - (n - K).
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
extern int *D_8009D12C;
extern int D_800A22B0[];
extern int D_800A2270[];
extern void func_800527C0(int);
extern unsigned char *func_8005DADC(int);
extern void func_8005FA3C(int);
extern void func_8005EB64(int);
extern void func_80077C84(void *a0, int a1, int a2, int a3);

static inline void push(void)
{
    int *r;
    int t0;
    int t1;

    r = D_8009D12C;
    if (r < D_800A22B0) {
        t0 = D_8009D124;
        t1 = D_8009D128;
        D_8009D12C = r + 2;
        r[0] = t0;
        r[1] = t1;
    } else {
        func_800527C0(2);
    }
}

static inline void push2(void)
{
    int *r;
    int t0;
    int t1;

    r = D_8009D12C;
    if (r < D_800A22B0) {
        t0 = D_8009D124;
        t1 = D_8009D128;
        r[0] = t0;
        r[1] = t1;
        D_8009D12C = r + 2;
    } else {
        func_800527C0(2);
    }
}

static inline void pop(void)
{
    int *t;
    int t0;
    int t1;

    t = D_8009D12C;
    if (D_800A2270 < t) {
        t0 = t[-2];
        t1 = t[-1];
        D_8009D12C = t - 2;
        D_8009D124 = t0;
        D_8009D128 = t1;
    } else {
        func_800527C0(3);
    }
}

static inline void move_pen(int dx, int dy)
{
    D_8009D124 += dx;
    D_8009D128 += dy;
}

#define SEG(X, W, U)                                                        \
    {                                                                       \
        PolyFT4 *g = 0;                                                     \
                                                                            \
        if (D_8009D100 + 10 < (unsigned int *)(D_8009D104 + 0x4000)) {      \
            D_8009D100 = D_8009D100 + 10;                                   \
            g = (PolyFT4 *)(D_8009D100 - 10);                               \
        } else {                                                            \
            func_800527C0(1);                                               \
        }                                                                   \
        if (g != 0) {                                                       \
            if (D_8009D10C != 0) {                                          \
                *(unsigned int *)&g->r0 = D_8009D114;                       \
            } else {                                                        \
                *(unsigned int *)&g->r0 = D_8009D110;                       \
            }                                                               \
            ((unsigned char *)g)[3] = 9;                                    \
            g->code = 0x2C;                                                 \
        }                                                                   \
        p = g;                                                              \
        p->x2 = p->x0 = (X);                                                \
        p->y0 = p->y1 = D_8009D128 + 1;                                     \
        p->x1 = p->x3 = p->x0 + (W);                                        \
        p->y2 = p->y3 = p->y0 + s[5];                                       \
        p->u0 = p->u2 = s[0] + (U);                                         \
        p->v0 = p->v1 = s[1];                                               \
        p->u1 = p->u3 = p->u0;                                              \
        p->v2 = p->v3 = p->v0 + s[5];                                       \
        p->clut = *(unsigned short *)(s + 2);                               \
        p->tpage = 7;                                                       \
        ((Tag *)p)->addr = ((Tag *)D_8009D11C)->addr;                       \
        ((Tag *)D_8009D11C)->addr = (unsigned int)p;                        \
    }

void func_8006062C(int a0, int n)
{
    PolyFT4 *p;
    unsigned char *s;
    void *m;

    s = func_8005DADC(0x48);
    push();
    move_pen(0, 2);
    push2();
    move_pen(0x27, -2);
    func_8005FA3C(a0);
    D_8009D110 = 0x808080;
    D_8009D114 = 0x404040;
    pop();
    func_8005EB64(0x49);
    if (n < 0x2F) SEG(D_8009D124 + 1, 1, 0)
    if (n < 0x2E) SEG(D_8009D124 + 2, 0x2E - n, 1)
    if (n < 0x30) SEG(D_8009D124 - (n - 0x30), 1, 2)
    if (n > 0) SEG(D_8009D124 - (n - 0x31), 1, 3)
    if (n >= 3) SEG(D_8009D124 - (n - 0x32), n - 1, 4)
    if (n >= 2) SEG(D_8009D124 + 0x31, 1, 5)
    pop();
    {
        int t = s[6];

        m = 0;
        if (D_8009D100 + 2 < (unsigned int *)(D_8009D104 + 0x4000)) {
            D_8009D100 = D_8009D100 + 2;
            m = D_8009D100 - 2;
        } else {
            func_800527C0(1);
        }
        if (m != 0) {
            func_80077C84(m, 0, 0, ((t & 3) << 7) | 7);
        }
        ((Tag *)m)->addr = ((Tag *)D_8009D11C)->addr;
        ((Tag *)D_8009D11C)->addr = (unsigned int)m;
    }
}
