/* ovl_0700 (PE.IMG map-exit overlay, VRAM 0x8018EFF0)
 * func_8018F92C — blob offset 0x93C, 0x6C8 bytes. Profile era_o2_g0 (default); LINK_EXACT at the overlay VMA.
 * Push the GTE matrix, load D_8019CDF0 as rot+trans, RotTrans four SVECTOR corners (D_8019BFD0..E8),
 * build four plane normals with OuterProduct0 of corner-minus-origin pairs, then store each plane's
 * distance (-n.p), width (n.q + d) and normal length (SquareRoot0(n.n)); pop the matrix. */
typedef struct { short vx, vy, vz, pad; } SVECTOR;
typedef struct { long vx, vy, vz, pad; } VECTOR;
typedef struct { short m[3][3]; long t[3]; } MATRIX;

extern MATRIX D_8019CDF0;
extern SVECTOR D_8019BFD0, D_8019BFD8, D_8019BFE0, D_8019BFE8;
extern VECTOR D_8019CBB0, D_8019CBD0, D_8019CBF0, D_8019CB50;
extern long D_8019CB48, D_8019CB4C, D_8019CBA8, D_8019CA90;
extern long D_8019CC04, D_8019CC0C, D_8019CC10, D_8019CBC4;
extern long D_8019CBC8, D_8019CC00, D_8019CC08, D_8019CBAC;

extern void func_80078A94();
extern void func_80078E94();
extern void func_80078E04();
extern void func_800792D4();
extern void func_800791D0();
extern long func_80078004();
extern void func_80078B38();

void func_8018F92C(VECTOR *o)
{
    VECTOR v0, v1, v2, v3;
    VECTOR a, b;
    long flag;

    func_80078A94();
    func_80078E94(&D_8019CDF0);
    func_80078E04(&D_8019CDF0);
    func_800792D4(&D_8019BFD0, &v0, &flag);
    func_800792D4(&D_8019BFD8, &v1, &flag);
    func_800792D4(&D_8019BFE0, &v2, &flag);
    func_800792D4(&D_8019BFE8, &v3, &flag);

    a.vx = v2.vx - o->vx; a.vy = v2.vy - o->vy; a.vz = v2.vz - o->vz;
    b.vx = v0.vx - o->vx; b.vy = v0.vy - o->vy; b.vz = v0.vz - o->vz;
    func_800791D0(&a, &b, &D_8019CBB0);
    a.vx = v3.vx - o->vx; a.vy = v3.vy - o->vy; a.vz = v3.vz - o->vz;
    b.vx = v1.vx - o->vx; b.vy = v1.vy - o->vy; b.vz = v1.vz - o->vz;
    func_800791D0(&a, &b, &D_8019CBD0);
    a.vx = v3.vx - o->vx; a.vy = v3.vy - o->vy; a.vz = v3.vz - o->vz;
    b.vx = v2.vx - o->vx; b.vy = v2.vy - o->vy; b.vz = v2.vz - o->vz;
    func_800791D0(&a, &b, &D_8019CBF0);
    a.vx = v1.vx - o->vx; a.vy = v1.vy - o->vy; a.vz = v1.vz - o->vz;
    b.vx = v0.vx - o->vx; b.vy = v0.vy - o->vy; b.vz = v0.vz - o->vz;
    func_800791D0(&a, &b, &D_8019CB50);

    D_8019CB48 = -(D_8019CBB0.vx * v2.vx) - D_8019CBB0.vy * v2.vy - D_8019CBB0.vz * v2.vz;
    D_8019CB4C = -(D_8019CBD0.vx * v3.vx) - D_8019CBD0.vy * v3.vy - D_8019CBD0.vz * v3.vz;
    D_8019CBA8 = -(D_8019CBF0.vx * v2.vx) - D_8019CBF0.vy * v2.vy - D_8019CBF0.vz * v2.vz;
    D_8019CA90 = -(D_8019CB50.vx * v1.vx) - D_8019CB50.vy * v1.vy - D_8019CB50.vz * v1.vz;
    D_8019CC04 = D_8019CBB0.vx * v1.vx + D_8019CBB0.vy * v1.vy + D_8019CBB0.vz * v1.vz + D_8019CB48;
    D_8019CC0C = D_8019CBD0.vx * v0.vx + D_8019CBD0.vy * v0.vy + D_8019CBD0.vz * v0.vz + D_8019CB4C;
    D_8019CC10 = D_8019CBF0.vx * v1.vx + D_8019CBF0.vy * v1.vy + D_8019CBF0.vz * v1.vz + D_8019CBA8;
    D_8019CBC4 = D_8019CB50.vx * v2.vx + D_8019CB50.vy * v2.vy + D_8019CB50.vz * v2.vz + D_8019CA90;
    D_8019CBC8 = func_80078004(D_8019CBB0.vx * D_8019CBB0.vx + D_8019CBB0.vy * D_8019CBB0.vy + D_8019CBB0.vz * D_8019CBB0.vz);
    D_8019CC00 = func_80078004(D_8019CBD0.vx * D_8019CBD0.vx + D_8019CBD0.vy * D_8019CBD0.vy + D_8019CBD0.vz * D_8019CBD0.vz);
    D_8019CC08 = func_80078004(D_8019CBF0.vx * D_8019CBF0.vx + D_8019CBF0.vy * D_8019CBF0.vy + D_8019CBF0.vz * D_8019CBF0.vz);
    D_8019CBAC = func_80078004(D_8019CB50.vx * D_8019CB50.vx + D_8019CB50.vy * D_8019CB50.vy + D_8019CB50.vz * D_8019CB50.vz);
    func_80078B38();
}
