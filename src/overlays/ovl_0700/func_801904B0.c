typedef struct { int vx, vy, vz, pad; } VECTOR;

extern int D_8019CBB0, D_8019CBB4, D_8019CBB8;
extern int D_8019CBD0, D_8019CBD4, D_8019CBD8;
extern int D_8019CBF0, D_8019CBF4, D_8019CBF8;
extern int D_8019CB50, D_8019CB54, D_8019CB58;
extern int D_8019CB48, D_8019CB4C, D_8019CBA8, D_8019CA90;
extern int D_8019CC04, D_8019CC0C, D_8019CC10, D_8019CBC4;
extern int D_8019CBC8, D_8019CC00, D_8019CC08, D_8019CBAC;

int func_801904B0(VECTOR *v, short r)
{
    int a, b, c, d;
    int f0, f1, f2, f3;
    int q;

    a = D_8019CBB0 * v->vx + D_8019CBB4 * v->vy + D_8019CBB8 * v->vz + D_8019CB48;
    b = D_8019CBD0 * v->vx + D_8019CBD4 * v->vy + D_8019CBD8 * v->vz + D_8019CB4C;
    c = D_8019CBF0 * v->vx + D_8019CBF4 * v->vy + D_8019CBF8 * v->vz + D_8019CBA8;
    d = D_8019CB50 * v->vx + D_8019CB54 * v->vy + D_8019CB58 * v->vz + D_8019CA90;
    f0 = f1 = f2 = f3 = 0;
    if (D_8019CC04 > 0 && a >= 0) f0 = 1;
    if (D_8019CC04 < 0 && a <= 0) f0 = 1;
    if (D_8019CC0C > 0 && b >= 0) f1 = 1;
    if (D_8019CC0C < 0 && b <= 0) f1 = 1;
    if (D_8019CC10 > 0 && c >= 0) f2 = 1;
    if (D_8019CC10 < 0 && c <= 0) f2 = 1;
    if (D_8019CBC4 > 0 && d >= 0) f3 = 1;
    if (D_8019CBC4 < 0 && d <= 0) f3 = 1;
    r = (r << 3) + 30;
    if (f0 == 0) {
        if (a >= 0) q = a / D_8019CBC8; else q = -a / D_8019CBC8;
        if (q < r) f0 = 1;
    }
    if (f1 == 0) {
        if (b >= 0) q = b / D_8019CC00; else q = -b / D_8019CC00;
        if (q < r) f1 = 1;
    }
    if (f2 == 0) {
        if (c >= 0) q = c / D_8019CC08; else q = -c / D_8019CC08;
        if (q < r) f2 = 1;
    }
    if (f3 == 0) {
        if (d >= 0) q = d / D_8019CBAC; else q = -d / D_8019CBAC;
        if (q < r) f3 = 1;
    }
    return f0 & f1 & f2 & f3;
}
