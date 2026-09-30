/* room_m0348i (PE.IMG room m0348i chunk 2, VRAM 0x8018EFE8)
 * func_8018FDCC — blob offset 0xde4, 0x31c bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0174i func_80190EE4; C re-targeted by symbol address
 * (docs/evidence/room_m0348i-ports-2026-09-23/REPORT.md). */

typedef struct { short m[3][3]; int t[3]; } MATRIX;
typedef struct { int vx, vy, vz, pad; } VECTOR;
typedef struct { short vx, vy, vz, pad; } SVECTOR;
typedef struct { unsigned char pad0[0x14]; short h14; short h16; } SPR;
extern SPR D_80192E50[2];
extern SVECTOR D_8018EFF4;
extern SVECTOR D_8018EFFC;
extern void func_800C3238();
extern void func_800794C4();
extern void func_80071A44();
extern void func_80078CC4();
extern void func_800C4FC4();

#define SH(o, x) (*(short *)((char *)(o) + (x)))

void func_8018FDCC(int a0, int a1, unsigned char *o)
{
    MATRIX m;
    SVECTOR r0, r1;
    MATRIX m2;
    VECTOR s;
    VECTOR v;
    VECTOR v2;

    r0 = D_8018EFF4;
    r1 = D_8018EFFC;
    func_800C3238(2);
    D_80192E50[0].h14 = SH(o, 0x14);
    D_80192E50[1].h14 = SH(o, 0x14);
    func_800794C4(&r1, &m);
    func_80071A44(&v, 0, 0x10);
    v.vx = SH(o, 0x10);
    v.vy = SH(o, 0x10);
    v.vz = SH(o, 0x10);
    s = v;
    func_80078CC4(&m, &s);
    m.t[0] = SH(o, 0);
    m.t[1] = SH(o, 2);
    m.t[2] = SH(o, 4);
    m2 = m;
    func_800C4FC4(&D_80192E50[0], &m, 0);
    m = m2;
    func_800C4FC4(&D_80192E50[1], &m, 0);
    func_800794C4(&r0, &m);
    func_80071A44(&v2, 0, 0x10);
    v2.vx = SH(o, 0x10) * 2;
    v2.vy = SH(o, 0x10) * 2;
    v2.vz = SH(o, 0x10) * 2;
    v = v2;
    func_80078CC4(&m, &v);
    m.t[0] = SH(o, 0);
    m.t[1] = SH(o, 2);
    m.t[2] = SH(o, 4);
    m2 = m;
    func_800C4FC4(&D_80192E50[0], &m, 0);
    m = m2;
    func_800C4FC4(&D_80192E50[1], &m, 0);
}
