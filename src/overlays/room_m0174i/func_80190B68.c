/* room_m0174i — func_80190B68, blob offset 0x1B80, 0x1E4 bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl5 2026-09-27).
 * Two-pass sprite draw (lit + floor shadow at D_800942EC); first draft exact. */

typedef struct { short m[3][3]; int t[3]; } MATRIX;
typedef struct { int vx, vy, vz, pad; } VECTOR;
typedef struct { short vx, vy, vz, pad; } SVECTOR;
typedef struct { unsigned char pad0[4]; unsigned char b4; unsigned char pad1[5]; short hA; } OBJ;
extern OBJ D_80197410;
extern SVECTOR D_8018F00C;
extern short D_800942EC;
extern unsigned char *func_800C2B50();
extern void func_800C2EAC();
extern void func_800C2FF0();
extern void func_800C3098();
extern void func_800C3238();
extern void func_800794C4();
extern void func_80071A44();
extern void func_80078CC4();
extern void func_800C42A4();

#define SH(o, x) (*(short *)((char *)(o) + (x)))

void func_80190B68(int a0, int a1, unsigned char *o)
{
    MATRIX m;
    SVECTOR r;
    VECTOR s;
    VECTOR v;
    VECTOR v2;
    unsigned char *x;

    x = func_800C2B50();
    r = D_8018F00C;
    func_800C2EAC(x[0x44]);
    func_800C2FF0(0x20, 0x20);
    func_800C3098(0x10);
    func_800C3238(2);
    func_800794C4(o + 8, &m);
    func_80071A44(&v, 0, 0x10);
    v.vx = SH(o, 0x10);
    v.vy = SH(o, 0x10);
    v.vz = 0x1000;
    s = v;
    func_80078CC4(&m, &s);
    D_80197410.hA = SH(o, 0x12);
    D_80197410.b4 = o[0x14] * 2 + 0x40;
    m.t[0] = SH(o, 0);
    m.t[1] = SH(o, 2);
    m.t[2] = SH(o, 4);
    func_800C42A4(&D_80197410, &m, 1);
    func_800794C4(&r, &m);
    func_80071A44(&v2, 0, 0x10);
    v2.vx = SH(o, 0x10);
    v2.vy = SH(o, 0x10);
    v2.vz = 0x1000;
    v = v2;
    func_80078CC4(&m, &v);
    D_80197410.hA = SH(o, 0x12) >> 2;
    m.t[0] = SH(o, 0);
    m.t[1] = D_800942EC;
    m.t[2] = SH(o, 4);
    func_800C42A4(&D_80197410, &m, 0);
}
