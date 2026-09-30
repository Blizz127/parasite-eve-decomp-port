/* room_m0414i (PE.IMG room m0414i chunk 2, VRAM 0x8018EFE8)
 * func_80190244 — blob offset 0x125c, 0x1f8 bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0141i func_80190278; C re-targeted by symbol address
 * (docs/evidence/room_m0414i-ports-2026-09-23/REPORT.md). */

typedef struct { short m[3][3]; int t[3]; } MATRIX;
typedef struct { int vx, vy, vz, pad; } VECTOR;
typedef struct { short vx, vy, vz, pad; } SVECTOR;
typedef struct { unsigned char pad0[4]; unsigned char b4; unsigned char b5; unsigned char pad1[4]; short hA; } OBJ;
extern OBJ D_80192170;
extern VECTOR D_8018F034;
extern short D_800942EC;
extern unsigned char *func_800C2B50();
extern void func_800C2EAC();
extern void func_800C2FF0();
extern void func_800C3098();
extern void func_800C3238();
extern void func_80078CC4();
extern void func_800C42A4();

void func_80190244(int a0, int a1, unsigned char *a2)
{
    MATRIX m;
    VECTOR s;
    unsigned int i;
    SVECTOR *p;
    unsigned char *x;
    int c;

    x = func_800C2B50();
    asm volatile("");
    func_800C2EAC(x[0x24]);
    func_800C2FF0(0x20, 0x20);
    func_800C3098(0x10);
    func_800C3238(2);
    m.m[0][0] = m.m[1][1] = m.m[2][2] = 0x1000;
    m.t[0] = m.t[1] = m.t[2] = 0;
    m.m[0][1] = m.m[0][2] = m.m[1][0] = m.m[1][2] = m.m[2][0] = m.m[2][1] = 0;
    s = D_8018F034;
    func_80078CC4(&m, &s);
    p = (SVECTOR *)a2;
    c = 0x80;
    for (i = 0; i < 6; i++) {
        c -= 0x14;
        if (((short *)(a2 + 0x78))[i] == 1) {
            m.t[0] = p[i].vx;
            m.t[1] = p[i].vy;
            m.t[2] = p[i].vz;
            if (i == 0) {
                func_800C3238(2);
                D_80192170.b4 = 0x42;
                D_80192170.b5 = 3;
                D_80192170.hA = 0xFF;
            } else {
                func_800C3238(3);
                D_80192170.b4 = 0x47;
                D_80192170.b5 = 5;
                D_80192170.hA = c;
            }
            func_800C42A4(&D_80192170, &m, 1);
            if (i == 0) {
                m.t[0] = *(short *)a2;
                m.t[1] = D_800942EC;
                m.t[2] = *(short *)(a2 + 4);
                D_80192170.hA = 0x40;
                D_80192170.b4 = 0x42;
                D_80192170.b5 = 3;
                func_800C3238(2);
                func_800C42A4(&D_80192170, &m, 1);
            }
        }
    }
}
