/* room_m0187i (PE.IMG room m0187i chunk 2, VRAM 0x8018EFE8)
 * func_8018F900 — blob offset 0x918, 0x190 bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0186i func_80190774; C re-targeted by symbol address
 * (docs/evidence/room_m0187i-ports-2026-09-23/REPORT.md). */

typedef struct { short m[3][3]; int t[3]; } MATRIX;
typedef struct { int vx, vy, vz, pad; } VECTOR;
typedef struct { short vx, vy, vz, pad; } SVECTOR;
typedef struct { unsigned char pad0[4]; unsigned char b4; unsigned char pad1[5]; short hA; } OBJ;
extern OBJ D_801900A0;
extern OBJ D_801900B0;
extern short D_800942EC;
extern unsigned char *func_800C2B50();
extern void func_800C2EAC();
extern void func_800C2FF0();
extern void func_800C3098();
extern void func_800C3238();
extern void func_800794C4();
extern void func_80071A44();
extern void func_80078CC4();
extern void func_800C42A4();

void func_8018F900(int a0, int a1, unsigned char *a2)
{
    MATRIX m;
    VECTOR s;
    VECTOR v;
    unsigned int i;
    SVECTOR *p;
    unsigned char *x;

    x = func_800C2B50();
    asm volatile("");
    func_800C2EAC(x[0x24]);
    func_800C2FF0(0x20, 0x20);
    func_800C3098(0x10);
    func_800C3238(2);
    p = (SVECTOR *)a2;
    for (i = 0; i < 8; i++) {
        if ((a2 + i)[0xC0] == 1) {
            func_800794C4((SVECTOR *)(a2 + 0x80) + i, &m);
            func_80071A44(&v, 0, 0x10);
            v.vx = *(short *)(a2 + 0xD0);
            v.vy = *(short *)(a2 + 0xD0);
            v.vz = 0x400;
            s = v;
            func_80078CC4(&m, &s);
            m.t[0] = p[i].vx;
            m.t[1] = p[i].vy;
            m.t[2] = p[i].vz;
            D_801900A0.b4 = (a2 + i)[0xC8] * 2 + 0x20;
            func_800C42A4(&D_801900A0, &m, 1);
            m.t[0] = p[i].vx;
            m.t[1] = D_800942EC;
            m.t[2] = p[i].vz;
            D_801900B0.b4 = (a2 + i)[0xC8] * 2 + 0x20;
            func_800C42A4(&D_801900B0, &m, 0);
        }
    }
}
