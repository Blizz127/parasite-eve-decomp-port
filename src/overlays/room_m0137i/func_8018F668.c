/* room_m0137i (PE.IMG room m0137i chunk 2, VRAM 0x8018EFE8)
 * func_8018F668 — blob offset 0x680, 0x1D0 bytes. Profile era_o2_g0 (default);
 * LINK_EXACT at the room VMA (docs/evidence/room-lane-2026-09-23/REPORT.md).
 * Draw a stretched sprite (+0x10 height) and its floor shadow at D_800942EC. */

typedef struct { short m[3][3]; int t[3]; } MATRIX;
typedef struct { int vx, vy, vz, pad; } VECTOR;
typedef struct { unsigned char pad[0xA]; short fA; } Lt;
extern unsigned char *func_800C2B50();
extern void func_800C2EAC();
extern void func_800C2FF0();
extern void func_800C3098();
extern void func_800C3238();
extern void func_80071A44();
extern void func_80078CC4();
extern void func_800C42A4();
extern VECTOR D_8018F004;
extern Lt D_80190F70;
extern Lt D_80190F50;
extern short D_800942EC;
void func_8018F668(int a0, int a1, short *p)
{
    MATRIX m;
    VECTOR s2;
    VECTOR s;

    func_800C2EAC(func_800C2B50()[0x10]);
    func_800C2FF0(16, 16);
    func_800C3098(16);
    func_800C3238(0);
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
    s.vx = 256;
    s.vy = p[8];
    s.vz = 4096;
    s2 = s;
    func_80078CC4(&m, &s2);
    D_80190F70.fA = p[9];
    m.t[0] = p[0];
    m.t[1] = p[1] - ((p[8] - 256) >> 4);
    m.t[2] = p[2];
    func_800C42A4(&D_80190F70, &m, 1);
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
    s = D_8018F004;
    func_80078CC4(&m, &s);
    func_800C3238(2);
    m.t[0] = p[0];
    m.t[1] = D_800942EC;
    m.t[2] = p[2];
    func_800C42A4(&D_80190F50, &m, 1);
}
