/* room_m0392i (PE.IMG room m0392i chunk 2, VRAM 0x8018EFE8)
 * func_8018FBD4 — blob offset 0xbec, 0x12c bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0137i func_8018FA2C; C re-targeted by symbol address
 * (docs/evidence/room_m0392i-ports-2026-09-23/REPORT.md). */

typedef struct { short m[3][3]; int t[3]; } MATRIX;
typedef struct { int vx, vy, vz, pad; } VECTOR;
typedef struct { short vx, vy, vz, pad; } SVECTOR;
typedef struct { unsigned char pad[4]; unsigned char b4; unsigned char pad5[5]; short fA; } Lt;
extern unsigned char *func_800C2B50();
extern void func_800C2EAC();
extern void func_800C2FF0();
extern void func_800C3098();
extern void func_800C3238();
extern void func_800794C4();
extern void func_80071A44();
extern void func_80078CC4();
extern void func_800C42A4();
extern SVECTOR D_8018F014;
extern Lt D_801946D0;
void func_8018FBD4(int a0, int a1, short *p)
{
    unsigned char *r = func_800C2B50();
    MATRIX m;
    SVECTOR rot;
    VECTOR s2;
    VECTOR s;

    rot = D_8018F014;
    func_800C2EAC(r[0x10]);
    func_800C2FF0(32, 32);
    func_800C3098(16);
    func_800C3238(1);
    func_800794C4(&rot, &m);
    func_80071A44(&s, 0, 16);
    s.vx = p[8];
    s.vy = p[8];
    s.vz = 4096;
    s2 = s;
    func_80078CC4(&m, &s2);
    D_801946D0.fA = p[9];
    m.t[0] = p[0];
    m.t[1] = p[1];
    m.t[2] = p[2];
    D_801946D0.b4 = ((unsigned char *)p)[0x15] * 2;
    func_800C42A4(&D_801946D0, &m, 0);
}
