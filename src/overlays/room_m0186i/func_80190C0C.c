/* room_m0186i (PE.IMG room m0186i chunk 2, VRAM 0x8018EFE8)
 * func_80190C0C — blob offset 0x1C24, 0x130 bytes. Profile era_o2_g0 (default);
 * LINK_EXACT at the room VMA (docs/evidence/room-lane-2026-09-23/REPORT.md).
 * Draw setup: identity matrix scaled (+0x10, +0x10, 1024), translate +0, brightness from +0x16 into D_80194350. */

typedef struct { short m[3][3]; int t[3]; } MATRIX;
typedef struct { int vx, vy, vz, pad; } VECTOR;
typedef struct { unsigned char pad[4]; unsigned char b4; unsigned char pad5[5]; short fA; } Lt;
extern unsigned char *func_800C2B50();
extern void func_800C2EAC();
extern void func_800C2FF0();
extern void func_800C3098();
extern void func_800C3238();
extern void func_80071A44();
extern void func_80078CC4();
extern void func_800C42A4();
extern Lt D_80194350;
void func_80190C0C(int a0, int a1, short *p)
{
    MATRIX m;
    VECTOR s2;
    VECTOR s;

    func_800C2EAC(func_800C2B50()[0x24]);
    func_800C2FF0(32, 32);
    func_800C3098(16);
    func_800C3238(2);
    m.m[2][2] = 4096;
    m.m[1][1] = 4096;
    m.m[0][0] = 4096;
    m.t[2] = 0;
    m.t[1] = 0;
    m.t[0] = 0;
    m.m[2][1] = 0;
    m.m[2][0] = 0;
    m.m[1][2] = 0;
    m.m[1][0] = 0;
    m.m[0][2] = 0;
    m.m[0][1] = 0;
    func_80071A44(&s, 0, 16);
    s.vx = p[8];
    s.vy = p[8];
    s.vz = 1024;
    s2 = s;
    func_80078CC4(&m, &s2);
    m.t[0] = p[0];
    m.t[1] = p[1];
    m.t[2] = p[2];
    D_80194350.b4 = (p[11] >> 1) * 2;
    D_80194350.fA = p[9];
    func_800C42A4(&D_80194350, &m, 1);
}
