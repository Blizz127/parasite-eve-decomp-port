/* room_m0174i — func_801912E4, blob offset 0x22FC, 0x630 bytes. Flags -O2 -G0;
 * LINK_EXACT at the target VMA (lane ovl5 2026-09-27).
 * Spark draw (lit, shadow, glow + func_800C6xxx flare); (x%3<<1) not *2; pointer-arith hA store keeps D_800942EC load after it; hA,b4,b5 order; int t=(unsigned short)call. */

typedef struct { short m[3][3]; int t[3]; } MATRIX;
typedef struct { int vx, vy, vz, pad; } VECTOR;
typedef struct { short vx, vy, vz, pad; } SVECTOR;
typedef struct { unsigned char pad0[4]; unsigned char b4; unsigned char b5; unsigned char pad1[4]; short hA; } OBJ;
extern OBJ D_801974D0;
extern OBJ D_801973E8;
extern SVECTOR D_8018F00C;
extern SVECTOR D_8018F01C;
extern VECTOR D_8018F024;
extern VECTOR D_8018F034;
extern VECTOR D_8018F044;
extern short D_800942EC;
extern int D_80197404;
extern unsigned char *func_800C2B50();
extern void func_800C2EAC();
extern void func_800C2FF0();
extern void func_800C3098();
extern void func_800C3238();
extern void func_800794C4();
extern void func_80071A44();
extern void func_80078CC4();
extern void func_800C42A4();
extern void func_800C6D5C();
extern void func_800C6EE8();
extern int func_80077A64();
extern int func_80077AA4();
extern void func_800C6EC0();
extern void func_800C6ED8();
extern void func_800C6EF8();
extern void func_800C6FA0();
extern void func_800C71E4();
extern void func_800C6F4C();

#define SH(o, x) (*(short *)((char *)(o) + (x)))
#define UH(o, x) (*(unsigned short *)((char *)(o) + (x)))
#define W(o, x) (*(int *)((char *)(o) + (x)))

void func_801912E4(int a0, unsigned char *a1, unsigned char *o)
{
    SVECTOR r1;
    SVECTOR r0;
    MATRIX m;
    MATRIX m2;
    VECTOR sa;
    VECTOR sb;
    VECTOR v;
    unsigned char *x;
    unsigned int i;
    int t;

    x = func_800C2B50();
    r1 = D_8018F01C;
    r0 = D_8018F00C;
    func_800C2EAC(x[0x44]);
    func_800C2FF0(0x20, 0x20);
    func_800C3098(0x10);
    for (i = 0; i < SH(o, 0xE8); i++) {
        if ((o + i)[0xC8] == 1) {
            func_800C3238(2);
            r1.vz = SH(o + i * 2, 0x88);
            func_800794C4(&r1, &m);
            sa = D_8018F024;
            func_80078CC4(&m, &sa);
            D_801974D0.b4 = (SH(a1, 2) % 3 << 1) + 0x44;
            D_801974D0.b5 = 5;
            D_801974D0.hA = SH(o, 0xEA);
            m.t[0] = (W(o + i * 16, 8) >> 16) + SH(o, 0);
            m.t[1] = (W(o + i * 16, 0xC) >> 16) + SH(o, 2);
            m.t[2] = (W(o + i * 16, 0x10) >> 16) + SH(o, 4);
            func_800C42A4(&D_801974D0, &m, 1);
            D_801974D0.b5 = 9;
            func_800794C4(&r0, &m);
            sb = D_8018F024;
            func_80078CC4(&m, &sb);
            *(short *)((char *)&D_801974D0 + 0xA) = SH(o, 0xEA) >> 2;
            m.t[0] = (W(o + i * 16, 8) >> 16) + SH(o, 0);
            m.t[1] = D_800942EC;
            m.t[2] = (W(o + i * 16, 0x10) >> 16) + SH(o, 4);
            func_800C42A4(&D_801974D0, &m, 0);
            D_801974D0.b5 = 0xB;
        }
        if ((o + i)[0xC8] >= 2) {
            func_800C3238(2);
            if ((o + i)[0xC8] < 0xB) {
                func_800C3098(0x10);
                m.m[0][0] = m.m[1][1] = m.m[2][2] = 0x1000;
                m.t[0] = m.t[1] = m.t[2] = 0;
                m.m[0][1] = m.m[0][2] = m.m[1][0] = m.m[1][2] = m.m[2][0] = m.m[2][1] = 0;
                sa = D_8018F034;
                func_80078CC4(&m, &sa);
                D_801974D0.hA = SH(o, 0xEA);
                D_801974D0.b4 = (o + i)[0xC8] * 2 + 0x3A;
                D_801974D0.b5 = 5;
                m.t[0] = (W(o + i * 16, 8) >> 16) + SH(o, 0);
                m.t[1] = (W(o + i * 16, 0xC) >> 16) + SH(o, 2) - 200;
                m.t[2] = (W(o + i * 16, 0x10) >> 16) + SH(o, 4);
                func_800C42A4(&D_801974D0, &m, 1);
            }
            func_800C3098(0x100);
            m.m[0][0] = m.m[1][1] = m.m[2][2] = 0x1000;
            m.t[0] = m.t[1] = m.t[2] = 0;
            m.m[0][1] = m.m[0][2] = m.m[1][0] = m.m[1][2] = m.m[2][0] = m.m[2][1] = 0;
            sa = D_8018F044;
            func_80078CC4(&m, &sa);
            D_801973E8.hA = SH(o + i * 2, 0xD0);
            m.t[0] = (W(o + i * 16, 8) >> 16) + SH(o, 0);
            m.t[1] = (W(o + i * 16, 0xC) >> 16) + SH(o, 2) - 200;
            m.t[2] = (W(o + i * 16, 0x10) >> 16) + SH(o, 4);
            m2 = m;
            func_800C42A4(&D_801973E8, &m, 1);
            m = m2;
            m.t[1] += 200;
            func_80071A44(&v, 0, 0x10);
            v.vx = (o + i)[0xC8] * 32 + 0x20C;
            v.vy = (o + i)[0xC8] * 16;
            v.vz = (o + i)[0xC8] * 32 + 0x20C;
            sb = v;
            func_80078CC4(&m, &sb);
            func_800C6D5C(D_80197404, 0, 0);
            func_800C6EE8(-100);
            t = (unsigned short)func_80077A64(1, 1, 0x340, 0x100);
            func_800C6EC0(t, (unsigned short)func_80077AA4(0, 0x1DB));
            func_800C6ED8(1);
            func_800C6EF8(D_80197404);
            func_800C6FA0(D_80197404, (unsigned short)(UH(o + i * 2, 0xD0) * 2));
            func_800C71E4(D_80197404, &m);
            func_800C6F4C(D_80197404);
        }
    }
}
