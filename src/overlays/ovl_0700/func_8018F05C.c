typedef struct { short m[3][3]; int t[3]; } MATRIX;
typedef struct { int vx, vy, vz, pad; } VECTOR;
typedef struct { short vx, vy, vz, pad; } SVECTOR;

extern MATRIX D_8018EFF4;
extern VECTOR D_8019C810;
extern VECTOR D_8019C330;
extern SVECTOR D_8019BFC4;
extern MATRIX D_8019CC30;
extern MATRIX D_8019CDF0;
extern void func_80078134();
extern int func_80078004();
extern int func_80079FB4();
extern void func_800794C4();
extern void func_800787D4();
extern void func_8018F344();
extern void func_800799E4();
extern void func_80078E94();
extern void func_80078E04();

void func_8018F05C(void)
{
    VECTOR d;
    VECTOR n;
    MATRIX m1;
    MATRIX m2;
    SVECTOR s1;
    SVECTOR s2;
    VECTOR v;

    m1 = D_8018EFF4;
    m2 = D_8018EFF4;
    d.vx = D_8019C810.vx - D_8019C330.vx;
    d.vy = D_8019C810.vy - D_8019C330.vy;
    d.vz = D_8019C810.vz - D_8019C330.vz;
    func_80078134(&d, &n);
    m1.t[0] = -D_8019C330.vx;
    m1.t[1] = -D_8019C330.vy;
    m1.t[2] = -D_8019C330.vz;
    D_8019BFC4.vx = func_80079FB4(n.vy, func_80078004(n.vx * n.vx + n.vz * n.vz));
    D_8019BFC4.vy = func_80079FB4(n.vz, n.vx) - 0x400;
    D_8019BFC4.vz = 0;
    func_800794C4(&D_8019BFC4, &m2);
    func_800787D4(&m2, &m1, &D_8019CC30);
    v.vx = 0;
    v.vy = -20000;
    v.vz = 0;
    s1.vx = D_8019C810.vx;
    s1.vy = D_8019C810.vy;
    s1.vz = D_8019C810.vz;
    s2.vx = D_8019C330.vx;
    s2.vy = D_8019C330.vy;
    s2.vz = D_8019C330.vz;
    func_8018F344(&D_8019CC30, &s2, &s1, &v);
    D_8019BFC4.vx = -func_80079FB4(func_80078004(d.vx * d.vx + d.vz * d.vz), -d.vy) + 0x400;
    D_8019BFC4.vy = -func_80079FB4(d.vz, d.vx) + 0x400;
    D_8019BFC4.vz = 0;
    func_800799E4(&D_8019BFC4, &D_8019CDF0);
    D_8019CDF0.t[0] = D_8019C330.vx;
    D_8019CDF0.t[1] = D_8019C330.vy;
    D_8019CDF0.t[2] = D_8019C330.vz;
    func_80078E94(&D_8019CC30);
    func_80078E04(&D_8019CC30);
}
