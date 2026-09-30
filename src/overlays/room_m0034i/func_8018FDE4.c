/* room_m0034i (PE.IMG room m0034i chunk 2, VRAM 0x8018EFE8)
 * func_8018FDE4 — blob offset 0xdfc, 0xfc bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0174i func_8019002C; C re-targeted by symbol address
 * (docs/evidence/room_m0034i-ports-2026-09-23/REPORT.md). */

typedef struct { short m[3][3]; int t[3]; } MATRIX;
typedef struct { int vx, vy, vz, pad; } VECTOR;
typedef struct { unsigned char pad[0xA]; short fA; } Lt;
extern void func_800C2EAC();
extern void func_800C2FF0();
extern void func_800C3098();
extern void func_800C3238();
extern void func_80078CC4();
extern void func_800C42A4();
extern VECTOR D_8018EFFC;
extern Lt D_80190060;
void func_8018FDE4(int a0, int a1, short *p)
{
    MATRIX m;
    VECTOR s;

    func_800C2EAC(0);
    func_800C3098(16);
    func_800C2FF0(32, 32);
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
    D_80190060.fA = p[2];
    m.t[0] = p[4];
    m.t[1] = p[5];
    m.t[2] = p[6];
    s = D_8018EFFC;
    func_80078CC4(&m, &s);
    func_800C42A4(&D_80190060, &m, 1);
}
