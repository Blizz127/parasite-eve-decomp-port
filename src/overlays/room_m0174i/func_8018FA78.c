/* room_m0174i (PE.IMG room m0174i chunk 2, VRAM 0x8018EFE8)
 * func_8018FA78 — blob offset 0xa90, 0x424 bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0034i func_8018F830; C re-targeted by symbol address
 * (docs/evidence/room_m0174i-ports-2026-09-23/REPORT.md). */

typedef struct { short m[3][3]; int t[3]; } MATRIX;
typedef struct { int vx, vy, vz, pad; } VECTOR;
typedef struct { short vx, vy, vz, pad; } SVECTOR;
typedef struct { unsigned char r, g, b, pad3; unsigned char pad4[6]; short hA; } OBJ;
extern OBJ D_80197430;
extern unsigned char *D_8009D254;
extern void func_800C2EAC();
extern void func_800C3098();
extern void func_800C2FF0();
extern void func_800C3238();
extern void func_80071A44();
extern void func_80078CC4();
extern void func_800C42A4();
extern int func_800C61A8();
extern int func_800C6CE0();

#define SB(o, x) (*(signed char *)((char *)(o) + (x)))
#define SH(o, x) (*(short *)((char *)(o) + (x)))
#define W(o, x) (*(int *)((char *)(o) + (x)))
#define P(o, x) (*(void **)((char *)(o) + (x)))

void func_8018FA78(unsigned char *a0, unsigned char *a1, unsigned char *o)
{
    MATRIX m;
    SVECTOR sv;
    VECTOR s;
    VECTOR v;
    VECTOR v2;
    unsigned char *p;

    func_800C2EAC(0);
    func_800C3098(0x10);
    func_800C2FF0(0x40, 0x20);
    func_800C3238(2);
    if (SB(o, 0) == 1) {
        D_80197430.r = 0x10;
        D_80197430.g = 0x10;
        D_80197430.b = 0x20;
        D_80197430.hA = SH(o, 6);
        m = *(MATRIX *)(o + 0x24);
        m.t[0] = SH(o, 8) - SH(o, 0x18);
        m.t[1] = SH(o, 0xA) - SH(o, 0x18);
        m.t[2] = SH(o, 0xC) - SH(o, 0x18);
        func_80071A44(&v, 0, 0x10);
        v.vx = SH(o, 4) >> 1;
        v.vy = 0x251;
        v.vz = 0x251;
        s = v;
        func_80078CC4(&m, &s);
        func_800C42A4(&D_80197430, &m, 0);
        D_80197430.r = 0x20;
        D_80197430.g = 0x20;
        D_80197430.b = 0x40;
        m = *(MATRIX *)(o + 0x24);
        m.t[0] = SH(o, 8);
        m.t[1] = SH(o, 0xA);
        m.t[2] = SH(o, 0xC);
        func_80071A44(&v2, 0, 0x10);
        v2.vx = SH(o, 4);
        v2.vy = 0x448;
        v2.vz = 0x448;
        v = v2;
        func_80078CC4(&m, &v);
        func_800C42A4(&D_80197430, &m, 0);
        p = D_8009D254;
        sv.vx = W(p, 0x28) >> 16;
        sv.vy = W(p, 0x2C) >> 16;
        sv.vz = W(p, 0x30) >> 16;
        if (func_800C61A8(&sv, &m)) {
            if (func_800C6CE0(a0) == 3) {
                W(*(void **)D_8009D254, 0x4C) |= 0x4000;
                if (P(a0, 8)) {
                    **(int **)P(a0, 8) |= 0x80000000;
                }
                a1[1] = 2;
            }
        }
        D_80197430.r = 0x78;
        D_80197430.g = 0xF0;
        D_80197430.b = 0x78;
        m.t[1] = SH(o, 0xA) - 0x100;
        func_800C42A4(&D_80197430, &m, 0);
        m.t[0] = SH(o, 8) - SH(o, 0x18);
        m.t[1] = SH(o, 0xA) - SH(o, 0x1A);
        m.t[2] = SH(o, 0xC) - SH(o, 0x1C);
        D_80197430.r = 0x3C;
        D_80197430.g = 0x78;
        D_80197430.b = 0x3C;
        m.t[1] = SH(o, 0xA) - 0x100;
        func_800C42A4(&D_80197430, &m, 0);
        m.t[0] = SH(o, 8) - SH(o, 0x18) * 2;
        m.t[1] = SH(o, 0xA) - SH(o, 0x1A) * 2;
        m.t[2] = SH(o, 0xC) - SH(o, 0x1C) * 2;
        D_80197430.r = 0x1E;
        D_80197430.g = 0x3C;
        D_80197430.b = 0x1E;
        m.t[1] = SH(o, 0xA) - 0x100;
        func_800C42A4(&D_80197430, &m, 0);
    }
}
