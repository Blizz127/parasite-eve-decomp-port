/* room_m0428i — func_801924E8, blob offset 0x3500, 0x3E4 bytes. Flags -O2 -G0 (default
 * era_o2_g0, no knobs); LINK_EXACT at the target VMA (focus lane f10 2026-10-01). Plain C.
 * Glow sprite controller: mode 0 allocates the D_801941C0 sprite (func_8006E498) and seeds the
 * state (scale 0x400, ramp 0x40, angle step -0x200); mode 1 latches the owner's +0x18 flag
 * (1 -> 2, 4 when its animation +0xE == 7 reaches frame +0xF - 1, returning 1), integrates
 * angle/scale/height and fades +0xC by the D_800E27EC clock; mode 2 builds a Y-rotation
 * matrix, scales it, translates to the owner +0x1FC position, selects the texture page and
 * draws via func_800C6EF8/6FA0/71E4/6F4C.
 * Levers: owner pointer o loaded before the switch (retail $s2); `*b == mode` (retail compares
 * with the live mode register); chained zero stores in source order t0/t1/t2 and m01..m21
 * (cc1 emits chains in reverse); unsigned-short prototype for func_800C6FA0. */

#define B(o, x) (*(unsigned char *)((char *)(o) + (x)))
#define SH(o, x) (*(unsigned short *)((char *)(o) + (x)))
#define W(o, x) (*(int *)((char *)(o) + (x)))
#define P(o, x) (*(char **)((char *)(o) + (x)))
typedef struct { short m[3][3]; int t[3]; } MATRIX;
typedef struct { int vx, vy, vz, pad; } VECTOR;
extern int D_800B0E64;
extern int D_801941C0;
extern char *D_800F32D0;
extern int D_800E27EC;
extern unsigned short D_800E11FA;
extern unsigned short D_800E2850[];
extern unsigned short D_800F336C;
extern unsigned short D_800E1204[];
extern int D_800F3428;
extern int func_8006E498();
extern void func_800C6D5C();
extern int func_80077DC4();
extern int func_80077CF4();
extern void func_80071A44();
extern void func_80078CC4();
extern int func_80077A64();
extern int func_80077AA4();
extern void func_800C6EC0(unsigned short, unsigned short);
extern void func_800C6ED8();
extern void func_800C6EF8();
extern void func_800C6FA0(int, unsigned short);
extern void func_800C71E4();
extern void func_800C6F4C();

int func_801924E8(int mode, short *p)
{
    MATRIX m;
    VECTOR s2;
    VECTOR s;
    char *o;
    unsigned char *b;
    int c, sn;
    int u;
    unsigned short k;
    int w;
    int t;

    o = P(D_800F32D0, 8);
    switch (mode) {
    case 0:
        D_801941C0 = func_8006E498(D_800B0E64, 0xC54C7704);
        func_800C6D5C(D_801941C0, 0, 0);
        p[0] = 0x400;
        p[3] = 0x40;
        p[1] = 0;
        p[2] = 0;
        p[4] = 0;
        p[5] = -0x200;
        p[6] = 0;
        break;
    case 1:
        b = (unsigned char *)P(P(o, 0), 0x18);
        if (*b == mode)
            *b = 2;
        if (B(o, 0xE) == 7 && SH(o, 0x16) >= B(o, 0xF) - 1U) {
            *P(P(o, 0), 0x18) = 4;
            return 1;
        }
        p[4] += p[5];
        if (D_800E27EC >= 9)
            p[5] -= 8;
        p[0] += p[2];
        if (p[0] < 0x40)
            p[0] = 0x40;
        if (D_800E27EC >= 9)
            p[2] -= 2;
        p[1] += p[3];
        p[3] += 6;
        t = D_800E27EC;
        if (t < 0x10) {
            p[6] += 8;
            break;
        }
        if (t < 0x13)
            break;
        if (p[6] != 0) {
            p[6] -= 5;
            if (p[6] < 0)
                p[6] = 0;
        }
        break;
    case 2:
        if (p[6] == 0)
            break;
        c = func_80077DC4(p[4]);
        sn = func_80077CF4(p[4]);
        m.m[0][2] = sn;
        m.m[2][0] = -sn;
        m.m[0][0] = c;
        m.m[2][2] = c;
        m.t[0] = m.t[1] = m.t[2] = 0;
        m.m[0][1] = m.m[1][0] = m.m[1][2] = m.m[2][1] = 0;
        m.m[1][1] = 0x1000;
        func_80071A44(&s, 0, 16);
        s.vx = p[0];
        s.vy = p[1];
        s.vz = p[0];
        s2 = s;
        func_80078CC4(&m, &s2);
        m.t[0] = W(o, 0x1FC);
        m.t[1] = W(o, 0x200);
        m.t[2] = W(o, 0x204);
        u = (D_800E2850[D_800E11FA] | func_80077A64(0, 1, 0, 0)) & 0xFFFF;
        k = D_800F336C;
        w = D_800E1204[k] + ((k == 4 && D_800F3428 != 0) ? 8 : 4);
        func_800C6EC0(u, func_80077AA4(0, w));
        func_800C6ED8(1);
        func_800C6EF8(D_801941C0);
        func_800C6FA0(D_801941C0, p[6]);
        func_800C71E4(D_801941C0, &m);
        func_800C6F4C(D_801941C0);
        break;
    }
    return 0;
}
