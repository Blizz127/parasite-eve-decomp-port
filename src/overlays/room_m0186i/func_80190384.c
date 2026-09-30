/* room_m0186i — func_80190384, blob offset 0x139C, 0x158 bytes. Profile era_o2_g0 (default).
 * LINK_EXACT at the target VMA (lane ovl3 2026-09-27). The empty asm volatile after the first
 * call is a zero-code sched1 barrier: retail's loop setup (i = 0, &D_80194370+10) is not hoisted
 * above func_800C2B50. */

typedef struct { short m[3][3]; int t[3]; } MATRIX;
typedef struct { int vx, vy, vz, pad; } VECTOR;
typedef struct { short vx, vy, vz, pad; } SVECTOR;
typedef struct { unsigned char pad0[4]; unsigned char b4; unsigned char pad1[5]; short hA; } OBJ;
extern OBJ D_80194370;
extern unsigned char *func_800C2B50();
extern void func_800C2EAC();
extern void func_800C2FF0();
extern void func_800C3098();
extern void func_800C3238();
extern void func_80071A44();
extern void func_80078CC4();
extern void func_800C42A4();

void func_80190384(int a0, int a1, unsigned char *a2)
{
    MATRIX m;
    VECTOR s;
    VECTOR v;
    unsigned int i;
    SVECTOR *p;
    unsigned char *x;

    x = func_800C2B50();
    asm volatile("");
    func_800C2EAC(x[0x24]);
    func_800C2FF0(0x20, 0x20);
    func_800C3098(0x10);
    func_800C3238(2);
    m.m[0][0] = m.m[1][1] = m.m[2][2] = 0x1000;
    m.t[0] = m.t[1] = m.t[2] = 0;
    m.m[0][1] = m.m[0][2] = m.m[1][0] = m.m[1][2] = m.m[2][0] = m.m[2][1] = 0;
    func_80071A44(&v, 0, 0x10);
    v.vx = *(short *)(a2 + 0x80);
    v.vy = *(short *)(a2 + 0x80);
    v.vz = 0x400;
    s = v;
    func_80078CC4(&m, &s);
    p = (SVECTOR *)a2;
    for (i = 0; i < 8; i++) {
        m.t[0] = p[i].vx;
        m.t[1] = p[i].vy;
        m.t[2] = p[i].vz;
        D_80194370.hA = *(short *)(a2 + 0x82) >> 1;
        D_80194370.b4 = a2[0x84] << 1;
        func_800C42A4(&D_80194370, &m, 1);
    }
}
