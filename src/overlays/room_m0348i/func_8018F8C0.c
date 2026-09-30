/* room_m0348i (PE.IMG room m0348i chunk 2, VRAM 0x8018EFE8)
 * func_8018F8C0 — blob offset 0x8d8, 0xec bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0174i func_801909D8; C re-targeted by symbol address
 * (docs/evidence/room_m0348i-ports-2026-09-23/REPORT.md). */

typedef struct { short m[3][3]; int t[3]; } MATRIX;
typedef struct { int vx, vy, vz, pad; } VECTOR;
typedef struct { unsigned char pad[0xA]; short fA; } Lt;
extern void *func_800C2B50();
extern void func_800C2EAC();
extern void func_800C2FF0();
extern void func_800C3098();
extern void func_800C3238();
extern void func_800794C4();
extern void func_80071A44();
extern void func_80078CC4();
extern void func_800C42A4();
extern Lt D_80192E00;
void func_8018F8C0(int a0, int a1, short *p)
{
    MATRIX m;
    VECTOR s2;
    VECTOR s;

    func_800C2EAC(((unsigned char *)func_800C2B50())[0x44]);
    func_800C2FF0(32, 32);
    func_800C3098(256);
    func_800C3238(2);
    func_800794C4(p + 4, &m);
    func_80071A44(&s, 0, 16);
    s.vx = p[8];
    s.vy = p[8];
    s.vz = 4096;
    s2 = s;
    func_80078CC4(&m, &s2);
    D_80192E00.fA = p[9];
    m.t[0] = p[0];
    m.t[1] = p[1];
    m.t[2] = p[2];
    func_800C42A4(&D_80192E00, &m, 1);
}
