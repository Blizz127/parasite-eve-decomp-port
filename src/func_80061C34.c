typedef struct Tag {
    unsigned int addr : 24;
    unsigned int len : 8;
} Tag;

typedef struct Sprt {
    unsigned int tag;
    unsigned char r0, g0, b0, code;
    short x0, y0;
    unsigned char u0, v0;
    unsigned short clut;
    short w, h;
} Sprt;

extern int D_8009D124;
extern int D_8009D128;
extern short *D_8009D148;
extern short D_800A22B0[];
extern unsigned char D_800930A8[];
extern unsigned int *D_8009D100;
extern unsigned int D_8009D104;
extern int D_8009D10C;
extern unsigned int D_8009D110;
extern unsigned int D_8009D114;
extern unsigned int *D_8009D11C;
extern void func_800527C0(int);
extern void func_80061878(unsigned char *, int);
extern void func_80075B4C(void *, short *);
extern void func_80077C84(void *a0, int a1, int a2, int a3);

static inline void push(int x, int y)
{
    short *p;

    p = D_8009D148;
    if (p < D_800A22B0 + 0x18) {
        p[1] = x;
        D_8009D148 = p + 2;
        p[0] = y;
    } else {
        func_800527C0(4);
    }
}

void func_80061C34(int w, int h, unsigned char *p, int a3)
{
    short *q;
    int x;
    int y;
    unsigned char *s;
    short rect[4];
    short *rp;
    short *rq;
    void *m;
    void *mm;
    Sprt *sp;

    if (p != 0) {
        unsigned char *t = p;

        D_8009D148 = D_800A22B0;
        while (t[0] < 0xFF) {
            x = t[0] + D_8009D124;
            y = D_8009D128 + t[1];
            q = D_8009D148;
            if (q < D_800A22B0 + 0x18) {
                q[1] = x;
                D_8009D148 = q + 2;
                q[0] = y;
            } else {
                func_800527C0(4);
            }
            t += 2;
        }
        s = t + 1;
    } else {
        D_8009D148 = D_800A22B0;
        push(D_8009D124, D_8009D128);
        push(D_8009D124 + w, D_8009D128);
        push(D_8009D124, D_8009D128 + h);
        push(D_8009D124 + w, D_8009D128 + h);
        s = D_800930A8;
    }
    func_80061878(s, 0);
    if (a3 != 0) {
        rp = rect;
        rect[0] = 0;
        rect[1] = 0;
        rect[2] = 0;
        rect[3] = 0;
        m = 0;
        if (D_8009D100 + 3 < (unsigned int *)(D_8009D104 + 0x4000)) {
            D_8009D100 = D_8009D100 + 3;
            m = D_8009D100 - 3;
        } else {
            func_800527C0(1);
        }
        if (m != 0) {
            func_80075B4C(m, rp);
        }
        ((Tag *)m)->addr = ((Tag *)D_8009D11C)->addr;
        ((Tag *)D_8009D11C)->addr = (unsigned int)m;
        sp = 0;
        if (D_8009D100 + 5 < (unsigned int *)(D_8009D104 + 0x4000)) {
            D_8009D100 = D_8009D100 + 5;
            sp = (Sprt *)(D_8009D100 - 5);
        } else {
            func_800527C0(1);
        }
        if (sp != 0) {
            if (D_8009D10C != 0) {
                *(unsigned int *)&sp->r0 = D_8009D114;
            } else {
                *(unsigned int *)&sp->r0 = D_8009D110;
            }
            ((unsigned char *)sp)[3] = 4;
            sp->code = 0x64;
        }
        sp->x0 = D_8009D124;
        sp->clut = 0x391C;
        sp->w = w;
        sp->h = h;
        sp->y0 = D_8009D128;
        sp->u0 = 0;
        sp->v0 = 0;
        ((Tag *)sp)->addr = ((Tag *)D_8009D11C)->addr;
        ((Tag *)D_8009D11C)->addr = (unsigned int)sp;
        rq = rect;
        rect[2] = 0x20;
        rect[3] = 0x20;
        rect[0] = 0;
        rect[1] = 0;
        m = 0;
        if (D_8009D100 + 3 < (unsigned int *)(D_8009D104 + 0x4000)) {
            D_8009D100 = D_8009D100 + 3;
            m = D_8009D100 - 3;
        } else {
            func_800527C0(1);
        }
        if (m != 0) {
            func_80075B4C(m, rq);
        }
        ((Tag *)m)->addr = ((Tag *)D_8009D11C)->addr;
        ((Tag *)D_8009D11C)->addr = (unsigned int)m;
    }
    mm = 0;
    if (D_8009D100 + 2 < (unsigned int *)(D_8009D104 + 0x4000)) {
        D_8009D100 = D_8009D100 + 2;
        mm = D_8009D100 - 2;
    } else {
        func_800527C0(1);
    }
    if (mm != 0) {
        func_80077C84(mm, 0, 0, 7);
    }
    ((Tag *)mm)->addr = ((Tag *)D_8009D11C)->addr;
    ((Tag *)D_8009D11C)->addr = (unsigned int)mm;
}
