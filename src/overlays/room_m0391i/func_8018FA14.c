/* room_m0391i (PE.IMG room m0391i chunk 2, VRAM 0x8018EFE8)
 * func_8018FA14 — blob offset 0xa2c, 0x3a4 bytes. Profile era_o2_g0 (default).
 * Masked-body twin of room_m0065i func_8018F828; C re-targeted by symbol address
 * (docs/evidence/room_m0391i-ports-2026-09-23/REPORT.md). */

typedef struct { short m[3][3]; int t[3]; } MATRIX;
typedef struct { int vx, vy, vz, pad; } VECTOR;
typedef struct { short vx, vy, vz, pad; } SVECTOR;
typedef struct { unsigned char pad0[4]; unsigned char b4; unsigned char b5; unsigned char pad1[4]; short hA; } OBJ;
extern SVECTOR D_8018EFF4;
extern unsigned char D_801943B4[];
extern OBJ D_80194548;
extern OBJ D_80194558;
extern short D_800942EC;
extern unsigned char *func_800C2B50();
extern void func_800C2EAC();
extern void func_800C3098();
extern void func_80079754();
extern void func_80071A44();
extern void func_80078CC4();
extern void func_800C3134();
extern void func_800C2FF0();
extern void func_800C3238();
extern void func_800C42A4();
extern int func_80077CF4();
extern void func_800794C4();

#define SB(o, x) (*(signed char *)((char *)(o) + (x)))
#define UB(o, x) (*(unsigned char *)((char *)(o) + (x)))
#define SH(o, x) (*(short *)((char *)(o) + (x)))
#define UH(o, x) (*(unsigned short *)((char *)(o) + (x)))

void func_8018FA14(int a0, unsigned char *a1, unsigned char *o)
{
    MATRIX m;
    MATRIX m2;
    SVECTOR r0;
    SVECTOR r1;
    VECTOR s;
    VECTOR v;
    VECTOR s2;
    VECTOR v2;
    unsigned char *x;
    unsigned int i;
    short t;
    int q;

    x = func_800C2B50();
    r0 = D_8018EFF4;
    func_800C2EAC(x[0x2C]);
    func_800C3098(0x10);
    for (i = 0; i < 3; i++) {
        if (SB(o + i, 0x16) >= 0) {
            func_80079754((SVECTOR *)(o + 0x6C) + i, &m);
            func_80071A44(&v, 0, 0x10);
            v.vx = UH(o + i * 2, 0x2A) >> 1;
            v.vy = UH(o + i * 2, 0x2A) >> 1;
            v.vz = UH(o + i * 2, 0x2A) >> 1;
            s = v;
            func_80078CC4(&m, &s);
            m.t[0] = SH(o + i * 16, 0x3E) + SB(o, 0xB);
            m.t[1] = SH(o + i * 16, 0x42) + SB(o, 0xC);
            m.t[2] = SH(o + i * 16, 0x46) + SB(o, 0xD);
            D_80194558.hA = UB(o + i, 0x1E);
            func_800C3134(D_801943B4, SH(a1, 2), &D_80194558);
            func_800C2FF0(0x40, 0x40);
            func_800C3238(2);
            func_800C42A4(&D_80194558, &m, 1);
            if (SH(o, 8) == 1) {
                q = (func_80077CF4(SH(a1, 2) << 7) >> 3) + UH(o + i * 2, 0x32);
                t = q;
                if (UH(o + i * 2, 0x2A) < (short)q) {
                    t = UH(o + i * 2, 0x2A);
                }
                r1 = ((SVECTOR *)(o + 0x6C))[i];
                t >>= 1;
                r1.vz += UH(o + i * 2, 0x22) << 4;
                func_80079754(&r1, &m);
                func_80071A44(&v, 0, 0x10);
                v.vx = t;
                v.vy = t;
                v.vz = t;
                func_80078CC4(&m, &v);
                m.t[0] = SH(o + i * 16, 0x3E) + SB(o, 0xB);
                m.t[1] = SH(o + i * 16, 0x42) + SB(o, 0xC);
                m.t[2] = SH(o + i * 16, 0x46) + SB(o, 0xD);
                func_800C42A4(&D_80194558, &m, 1);
                func_800794C4(&r0, &m2);
                func_80071A44(&v2, 0, 0x10);
                v2.vx = UH(o + i * 2, 0x2A);
                v2.vy = UH(o + i * 2, 0x2A);
                v2.vz = UH(o + i * 2, 0x2A);
                s2 = v2;
                func_80078CC4(&m2, &s2);
                m2.t[0] = SH(o + i * 16, 0x3E);
                m2.t[1] = D_800942EC;
                m2.t[2] = SH(o + i * 16, 0x46);
                D_80194548.hA = UB(o + i, 0x1E) >> 1;
                func_800C2FF0(0x20, 0x20);
                func_800C3238(3);
                func_800C42A4(&D_80194548, &m2, 0);
            }
        }
    }
}
