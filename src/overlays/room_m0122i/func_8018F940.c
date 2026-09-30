/* room_m0122i (PE.IMG room m0122i chunk 2, VRAM 0x8018EFE8)
 * func_8018F940 — blob offset 0x958, 0x1c4 bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0107i func_8018FB04; C re-targeted by symbol address
 * (docs/evidence/room_m0122i-ports-2026-09-23/REPORT.md). */

#define H(o, x) (*(short *)((char *)(o) + (x)))
#define W(o, x) (*(int *)((char *)(o) + (x)))
#define P(o, x) (*(void **)((char *)(o) + (x)))
typedef struct { short m[3][3]; int t[3]; } MATRIX;
typedef struct { int vx, vy, vz, pad; } VECTOR;
extern void *func_800C2B50();
extern void func_800794C4();
extern void func_80071A44();
extern void func_80078CC4();
extern void func_800C6D5C();
extern int func_80077A64();
extern int func_80077AA4();
extern void func_800C6EC0(unsigned short, unsigned short);
extern void func_800C6ED8();
extern void func_800C6EF8();
extern void func_800C7098();
extern void func_800C6FA0();
extern void func_800C71E4();
extern void func_800C6F4C();
extern int D_801910B4;
void func_8018F940(void *o, int a1, short *p)
{
    void *r = func_800C2B50();
    MATRIX m;
    VECTOR s2;
    VECTOR s;
    int tp;

    func_800794C4(p + 4, &m);
    m.t[0] = H(P(o, 0x8), 0x2A);
    m.t[1] = H(P(o, 0x8), 0x2E);
    m.t[2] = H(P(o, 0x8), 0x32);
    func_80071A44(&s, 0, 16);
    s.vx = p[8];
    s.vy = 200;
    s.vz = p[8];
    s2 = s;
    func_80078CC4(&m, &s2);
    func_800C6D5C(D_801910B4, 0, 0);
    if (W(r, 0x24) == 0) {
        func_800C6EC0(func_80077A64(0, 1, 832, 256), func_80077AA4(0, 471));
    }
    if (W(r, 0x24) == 1) {
        func_800C6EC0(func_80077A64(0, 1, 832, 352), func_80077AA4(0, 475));
    }
    func_800C6ED8(1);
    func_800C6EF8(D_801910B4);
    func_800C7098(D_801910B4, 96, 16, 128);
    func_800C6FA0(D_801910B4, (unsigned short)p[9]);
    func_800C71E4(D_801910B4, &m);
    func_800C6F4C(D_801910B4);
}
