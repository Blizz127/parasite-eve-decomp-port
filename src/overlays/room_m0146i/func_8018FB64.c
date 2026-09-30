/* room_m0146i (PE.IMG room m0146i chunk 2, VRAM 0x8018EFE8)
 * func_8018FB64 — blob offset 0xb7c, 0x25c bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0141i func_8018FB70; C re-targeted by symbol address
 * (docs/evidence/room_m0146i-ports-2026-09-23/REPORT.md). */

typedef struct { short m[3][3]; int t[3]; } MATRIX;
typedef struct { int vx, vy, vz, pad; } VECTOR;
typedef struct { short vx, vy, vz, pad; } SV;
typedef struct { unsigned char pad[4]; unsigned char b4; unsigned char pad5[5]; short va; } OBJ;
extern unsigned char *func_800C2B50();
extern void func_800C2EAC();
extern void func_800C2FF0();
extern void func_800C3098();
extern void func_800C3238();
extern void func_80078CC4();
extern void func_800C42A4();
extern VECTOR D_8018F00C;
extern VECTOR D_8018F01C;
extern short D_800942EC;
extern OBJ D_801926B8;

void func_8018FB64(void *a0, void *a1, unsigned char *a2)
{
    MATRIX m;
    VECTOR s;
    VECTOR u;
    unsigned char *r;
    unsigned int i;
    unsigned char *x;

    x = func_800C2B50();
    asm volatile("");
    asm("" : "=r"(r) : "0"(x));
    func_800C2EAC(r[0x24]);
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
    s = D_8018F00C;
    func_80078CC4(&m, &s);
    m.t[0] = *(int *)(r + 0x18);
    m.t[1] = *(int *)(r + 0x1C);
    m.t[2] = *(int *)(r + 0x20);
    D_801926B8.va = 0x40;
    D_801926B8.b4 = a2[0x104] * 2 + 0x20;
    func_800C42A4(&D_801926B8, &m, 1);
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
    u = D_8018F01C;
    func_80078CC4(&m, &u);
    for (i = 0; i < 16; i++) {
        m.t[0] = ((SV *)a2)[i].vx;
        m.t[1] = ((SV *)a2)[i].vy;
        m.t[2] = ((SV *)a2)[i].vz;
        D_801926B8.va = *(short *)(a2 + 0x102);
        D_801926B8.b4 = a2[0x104] * 2 + 0x20;
        func_800C42A4(&D_801926B8, &m, 1);
        m.t[0] = ((SV *)a2)[i].vx;
        m.t[1] = D_800942EC;
        m.t[2] = ((SV *)a2)[i].vz;
        D_801926B8.va = *(short *)(a2 + 0x102) >> 2;
        D_801926B8.b4 = a2[0x104] * 2 + 0x20;
        func_800C42A4(&D_801926B8, &m, 1);
    }
}
