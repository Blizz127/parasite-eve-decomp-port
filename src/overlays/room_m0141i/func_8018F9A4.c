/* room_m0141i (PE.IMG room m0141i chunk 2, VRAM 0x8018EFE8)
 * func_8018F9A4 — blob offset 0x9BC, 0xEC bytes. Profile era_o2_g0 (default);
 * LINK_EXACT at the room VMA (docs/evidence/room-lane-2026-09-23/REPORT.md).
 * Draw setup: identity MATRIX scaled by D_8018EFFC, translated to the func_800C2B50() position, then func_800C42A4. */

typedef struct { short m[3][3]; int t[3]; } MATRIX;
typedef struct { int vx, vy, vz, pad; } VECTOR;
extern void *func_800C2B50();
extern void func_800C2EAC();
extern void func_800C2FF0();
extern void func_800C3098();
extern void func_800C3238();
extern void func_80078CC4();
extern void func_800C42A4();
extern VECTOR D_8018EFFC;
extern short D_800942EC;
extern char D_801921A0[];
void func_8018F9A4(void)
{
    MATRIX m;
    VECTOR s;
    int *r = func_800C2B50();

    func_800C2EAC(((unsigned char *)r)[0x24]);
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
    s = D_8018EFFC;
    func_80078CC4(&m, &s);
    m.t[0] = r[6];
    m.t[1] = D_800942EC;
    m.t[2] = r[8];
    func_800C42A4(D_801921A0, &m, 1);
}
