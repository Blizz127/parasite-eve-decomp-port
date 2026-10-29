/* room_m0418i — func_80194DD8, blob offset 0x5DF0, 0x438 bytes. Flags -O2 -G0.
 * Two D_801994C8 model draws at the actor (*r)->+0x238 points +0x414 and +0x2D4
 * (scale p->h8), then a 7-point func_800D3114 strip between those points
 * offset by the p->+0xC..+0x28 shorts. */

typedef struct { short m[3][3]; int t[3]; } MATRIX;
typedef struct { int vx, vy, vz, pad; } VECTOR;
typedef struct { short vx, vy, vz, pad; } SV;
typedef struct { unsigned char r, g, b, cd; } CV;
typedef struct { unsigned char pad0[0xA]; short hA; } OBJ;
#define SH(o, x) (*(short *)((char *)(o) + (x)))
#define W(o, x) (*(int *)((char *)(o) + (x)))
extern CV D_8018F060, D_8018F064;
extern OBJ D_801994C8;
extern short D_800F3374;
extern char **func_800C2B50();
extern void func_800C2EAC(), func_800C2FF0(), func_800C3098(), func_800C3238();
extern void func_80071A44(), func_80078CC4(), func_800C42A4(), func_800D3114();
extern unsigned short func_80077A64();
extern unsigned short func_80077AA4();

void func_80194DD8(int a0, int a1, char *p)
{
    MATRIX m;
    SV pts[7];
    CV o1;
    CV o2;
    VECTOR a;
    VECTOR b;
    VECTOR c;
    char **r;
    char *e;
    short k;
    int t;

    r = func_800C2B50();
    e = *(char **)(*r + 0x238);
    o1 = D_8018F060;
    o2 = D_8018F064;
    k = 0x1000;
    func_800C2EAC(((unsigned char *)r)[0x24]);
    func_800C2FF0(0x20, 0x20);
    func_800C3098(0x10);
    func_800C3238(2);
    D_800F3374 = 0x1F4;
    D_801994C8.hA = SH(p, 0xA);
    m.m[0][0] = m.m[1][1] = m.m[2][2] = k;
    m.t[0] = m.t[1] = m.t[2] = 0;
    m.m[0][1] = m.m[0][2] = m.m[1][0] = m.m[1][2] = m.m[2][0] = m.m[2][1] = 0;
    func_80071A44(&b, 0, 16);
    b.vx = SH(p, 8);
    b.vy = SH(p, 8);
    b.vz = 0x1000;
    a = b;
    func_80078CC4(&m, &a);
    m.t[0] = W(e, 0x414);
    m.t[1] = W(e, 0x418);
    m.t[2] = W(e, 0x41C);
    func_800C42A4(&D_801994C8, &m, 1);
    m.m[0][0] = m.m[1][1] = m.m[2][2] = k;
    m.t[0] = m.t[1] = m.t[2] = 0;
    m.m[0][1] = m.m[0][2] = m.m[1][0] = m.m[1][2] = m.m[2][0] = m.m[2][1] = 0;
    func_80071A44(&c, 0, 16);
    c.vx = SH(p, 8);
    c.vy = SH(p, 8);
    c.vz = 0x1000;
    b = c;
    func_80078CC4(&m, &b);
    m.t[0] = W(e, 0x2D4);
    m.t[1] = W(e, 0x2D8);
    m.t[2] = W(e, 0x2DC);
    func_800C42A4(&D_801994C8, &m, 1);
    pts[0].vx = W(e, 0x414);
    pts[0].vy = W(e, 0x418);
    pts[0].vz = W(e, 0x41C);
    pts[1].vx = SH(p, 0xC) + W(e, 0x414);
    pts[1].vy = SH(p, 0xE) + W(e, 0x418);
    pts[1].vz = SH(p, 0x10) + W(e, 0x41C);
    pts[2].vx = SH(p, 0x12) + W(e, 0x414);
    pts[2].vy = SH(p, 0x14) + W(e, 0x418);
    pts[2].vz = SH(p, 0x16) + W(e, 0x41C);
    pts[3].vx = SH(p, 0x24) + ((W(e, 0x414) + W(e, 0x2D4)) >> 1);
    pts[3].vy = SH(p, 0x26) + ((W(e, 0x418) + W(e, 0x2D8)) >> 1);
    pts[3].vz = SH(p, 0x28) + ((W(e, 0x41C) + W(e, 0x2DC)) >> 1);
    pts[4].vx = SH(p, 0x18) + W(e, 0x2D4);
    pts[4].vy = SH(p, 0x1A) + W(e, 0x2D8);
    pts[4].vz = SH(p, 0x1C) + W(e, 0x2DC);
    pts[5].vx = SH(p, 0x1E) + W(e, 0x2D4);
    pts[5].vy = SH(p, 0x20) + W(e, 0x2D8);
    pts[5].vz = SH(p, 0x22) + W(e, 0x2DC);
    pts[6].vx = W(e, 0x2D4);
    pts[6].vy = W(e, 0x2D8);
    pts[6].vz = W(e, 0x2DC);
    t = func_80077A64(0, 0, 0x340, 0x100);
    func_800D3114(pts, 7, 300, 0x40, 0x80, 0x9C, 0x10, t,
                  func_80077AA4(0, 0x1D7), 0x80, &o1, &o2, 1);
}
