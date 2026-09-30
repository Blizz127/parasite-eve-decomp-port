extern int D_8009D124;
extern int D_8009D128;
extern short *D_8009D148;
extern short D_800A22B0[];
extern unsigned char D_800930A8[];
extern void func_800527C0(int);
extern void func_80061878(unsigned char *, int);

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

void func_80061A3C(int w, int h, int a2)
{
    D_8009D148 = D_800A22B0;
    push(D_8009D124, D_8009D128);
    push(D_8009D124 + w, D_8009D128);
    push(D_8009D124, D_8009D128 + h);
    push(D_8009D124 + w, D_8009D128 + h);
    func_80061878(D_800930A8, a2);
}
