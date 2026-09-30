/* room_m0174i (PE.IMG room m0174i chunk 2, VRAM 0x8018EFE8)
 * func_8019002C — blob offset 0x1044, 0xFC bytes. Profile era_o2_g0 (default);
 * LINK_EXACT at the room VMA (docs/evidence/room-lane-2026-09-23/REPORT.md).
 * Draw setup: identity matrix scaled by D_8018EFFC, translated to +8..+0xC, func_800C42A4(&D_80197450). */

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
extern Lt D_80197450;
void func_8019002C(int a0, int a1, short *p)
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
    D_80197450.fA = p[2];
    m.t[0] = p[4];
    m.t[1] = p[5];
    m.t[2] = p[6];
    s = D_8018EFFC;
    func_80078CC4(&m, &s);
    func_800C42A4(&D_80197450, &m, 1);
}
