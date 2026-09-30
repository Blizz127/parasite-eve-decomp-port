/* room_m0146i (PE.IMG room m0146i chunk 2, VRAM 0x8018EFE8)
 * func_801912E8 — blob offset 0x2300, 0x188 bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0137i func_8018FE18; C re-targeted by symbol address
 * (docs/evidence/room_m0146i-ports-2026-09-23/REPORT.md). */

typedef struct { short m[3][3]; int t[3]; } MATRIX;
typedef struct { int vx, vy, vz, pad; } VECTOR;
typedef struct { short x, y, z, pad; } SV;
typedef struct {
    MATRIX m;
    SV pos[6];
    SV vel[6];
    unsigned char pad80[0xC];
    short size[6];
    short act[6];
    short count;
} Drops;
extern VECTOR D_8018F06C;
extern void func_80078CC4();
extern int func_80071A54();
void func_801912E8(int a0, int a1, Drops *p)
{
    VECTOR s;
    unsigned int i;

    p->m.m[2][2] = 4096;
    p->m.m[1][1] = 4096;
    p->m.m[0][0] = 4096;
    p->m.t[2] = 0;
    p->m.t[1] = 0;
    p->m.t[0] = 0;
    p->m.m[2][1] = 0;
    p->m.m[2][0] = 0;
    p->m.m[1][2] = 0;
    p->m.m[1][0] = 0;
    p->m.m[0][2] = 0;
    p->m.m[0][1] = 0;
    s = D_8018F06C;
    func_80078CC4(&p->m, &s);
    p->count = 6;
    for (i = 0; i < 6; i++) {
        p->act[i] = 1;
        p->size[i] = 128;
        p->vel[i].x = func_80071A54() % 31 - 15;
        p->vel[i].y = -(func_80071A54() % 100 + 50);
        p->vel[i].z = func_80071A54() % 31 - 15;
    }
}
