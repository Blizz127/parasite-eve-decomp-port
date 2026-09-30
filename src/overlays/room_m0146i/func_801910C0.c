/* room_m0146i (PE.IMG room m0146i chunk 2, VRAM 0x8018EFE8)
 * func_801910C0 — blob offset 0x20d8, 0x124 bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0137i func_8018FBF0; C re-targeted by symbol address
 * (docs/evidence/room_m0146i-ports-2026-09-23/REPORT.md). */

typedef struct { short m[3][3]; int t[3]; } MATRIX;
typedef struct { int vx, vy, vz, pad; } VECTOR;
typedef struct { short vx, vy, vz, pad; } SVECTOR;
typedef struct { unsigned char pad[0xA]; short fA; } Lt;
extern unsigned char *func_800C2B50();
extern void func_800C2EAC();
extern void func_800C2FF0();
extern void func_800C3098();
extern void func_800C3238();
extern void func_800794C4();
extern void func_80071A44();
extern void func_80078CC4();
extern void func_800C42A4();
extern SVECTOR D_8018F064;
extern Lt D_801926D8;
void func_801910C0(int a0, int a1, short *p)
{
    unsigned char *r = func_800C2B50();
    SVECTOR rot = D_8018F064;
    MATRIX m;
    VECTOR s2;
    VECTOR s;

    func_800C2EAC(r[0x10]);
    func_800C2FF0(64, 64);
    func_800C3098(16);
    func_800C3238(1);
    func_800794C4(&rot, &m);
    func_80071A44(&s, 0, 16);
    s.vx = p[4];
    s.vy = p[4];
    s.vz = p[4];
    s2 = s;
    func_80078CC4(&m, &s2);
    D_801926D8.fA = p[5];
    m.t[0] = p[0];
    m.t[1] = p[1];
    m.t[2] = p[2];
    func_800C42A4(&D_801926D8, &m, 0);
}
