extern short *D_8009D148;
extern int D_8009D124;
extern int D_8009D128;
extern short D_800A22B0[];
extern void func_800527C0(int);
extern void func_80061878(unsigned char *, int);

void func_80061B80(int a0, int a1, unsigned char *p)
{
    short *q;
    int x;
    int y;

    D_8009D148 = D_800A22B0;
    while (p[0] < 0xFF) {
        x = p[0] + D_8009D124;
        y = D_8009D128 + p[1];
        q = D_8009D148;
        if (q < D_800A22B0 + 0x18) {
            q[1] = x;
            D_8009D148 = q + 2;
            q[0] = y;
        } else {
            func_800527C0(4);
        }
        p += 2;
    }
    func_80061878(p + 1, 0);
}
