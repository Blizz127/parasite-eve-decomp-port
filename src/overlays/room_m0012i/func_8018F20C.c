/* room_m0012i (PE.IMG room m0012i chunk 2, VRAM 0x8018EFE8)
 * func_8018F20C — blob offset 0x224, 0xA48 bytes. Profile era_o2_g0 (default).
 * Shared room effect callback (cmd 0 init, cmd 1 a three-state update switch,
 * cmd 2 the D_800F3368 draw state plus sprites). It appears in 8 rooms. Twins
 * (masked-body ports): room_m0014i, m0113i, m0263i, m0412i, m0414i, m0421i, m0424i.
 * Levers (lanes sn-s658 → lane-s658b → lane-s658c; diffscore 174 → 0):
 *   - cmd 1 state 2 is `H(a1,2)--; if (SH(a1,0xA) < 0x10) break; return 1;`
 *     (the counter store in the bnez delay slot to the shared return 0);
 *   - an empty `do { } while (0);` opens draw case 0/1 (cse1 stops at its loop
 *     note but cse2 doesn't, so cmd 1 compares against $a3 while the draw
 *     argument keeps the constant-1 register);
 *   - the draw head's 0x10 constant goes through an empty register tie
 *     `__asm__("" : "=r"(c16) : "0"(c16))`, which stops dbr's target search so
 *     `li v1,16` fills the top-dispatch slot (8 words differ without it);
 *   - sp = &D_800F3368 is a single-set pointer and the first sprite's size is
 *     read as `(&D_800F3368)[1]`. cse keeps `lh 2(s1)` (not absolute), and sched2
 *     orders it after the sw24 stack store as in retail;
 *   - the head writes a70 before a6C, the first store through `*(short *)sp`,
 *     and the table read through H(D_800E2850, j * 2) (gcc 2.7 alias rule);
 *   - HS/SS struct-typed a1 accesses at two sites; both divides as explicit
 *     round-toward-zero fixups; cmd 0 ends in `return func_800CE560(...)`.
 * inline GTE/asm: the empty register tie above — count separately from plain C.
 * Hand-written in this repo (khasinski's vendored tree has this body as asm only). */
typedef struct { short x, y, z, w; } GteShortVector;

struct S { short a68, a6A; };
#define B(o, x) (*(unsigned char *)((char *)(o) + (x)))
#define H(o, x) (*(unsigned short *)((char *)(o) + (x)))
#define HS(o, x) (((struct { char p[x]; unsigned short v; } *)(o))->v)
#define SS(o, x) (((struct { char p[x]; short v; } *)(o))->v)
#define SH(o, x) (*(short *)((char *)(o) + (x)))
#define W(o, x) (*(int *)((char *)(o) + (x)))
#define P(o, x) (*(void **)((char *)(o) + (x)))

extern GteShortVector D_8018EFF4;
extern GteShortVector D_8018EFFC;
extern unsigned char func_8018F004[];
extern unsigned char *D_800E2368;
extern void *D_800F32D0;
extern void *D_800F33E0;
extern void **D_8009D254;
extern int D_800E27EC;
extern unsigned short D_800E11E8;
extern unsigned short D_800E1208;
extern unsigned short D_800E2850[];
extern unsigned short D_800E1204[];
extern int D_800F3428;
extern unsigned short D_800942EC;
extern int D_8009D248;
extern unsigned short D_8009D1CC;
extern short D_800F3368;
extern unsigned short D_800F336C;
extern short D_800F336A, D_800F336E, D_800F3370, D_800F3372, D_800F3374, D_800F3376, D_800F3378;

extern int func_800D3FD8();
extern int func_800D3F64();
extern int func_800CE560();
extern short *func_800CE610();
extern void func_800CE8F0();
extern void func_800CE9D4();
extern int func_80077CF4();
extern int func_80077DC4();
extern int func_80077AA4();
extern void func_800CEE20();
extern int func_8001CAB0();
extern int func_800C6B90();
extern void func_800866A4();

#define ASSIGN_W() do { w = D_800E1204[D_800F336C]; if (D_800F336C == 4 && D_800F3428 != 0) w += 4; } while (0)
#define COPYFX() do { v = func_800CE610(P(D_800F33E0, 8)); if (v != 0) { v[0] = H(a1, 0); v[1] = H(a1, 2); v[2] = H(a1, 4); W(v, 8) = SH(a1, 0x10); } } while (0)

int func_8018F20C(int a0, short *a1, int *a2)
{
    struct { GteShortVector a, b; } ee;
    struct S *sp;
    GteShortVector e3, e4, e5, e6;
    short *v;
    void *o;
    unsigned char *q;
    int mode;
    int k, w, t, s1, s2;


    ee.a = D_8018EFF4;
    ee.b = D_8018EFFC;

    mode = a0;
    switch (mode) {
    case 0:
        o = D_800F32D0;
        H(a1, 0) = H(P(o, 8), 0x268);
        H(a1, 2) = H(P(o, 8), 0x26A);
        q = D_800E2368;
        t = H(P(o, 8), 0x26C);
        H(a1, 8) = 0;
        H(a1, 0xA) = 0;
        SH(a1, 0xC) = -1;
        H(a1, 4) = t;
        switch (SH(q, 0x12)) {
        case 0:
            SH(a1, 0xE) = -a2[1];
            SH(a1, 0xC) = func_800D3F64(0x565, func_800D3FD8());
            break;
        case 1:
            SH(a1, 0xE) = 0;
            break;
        case 2:
            SH(a1, 0xE) = a2[1];
            break;
        }
        return func_800CE560(P(D_800F33E0, 8), 0xC, 0x10, func_8018F004);
    case 1:
        H(a1, 0xA) = H(a1, 0xA) + 1;
        switch (SH(a1, 8)) {
        case 0:
            if (a2[2] != 0) {
                func_800CE8F0(P(D_800F32D0, 8), 0, &ee.a, a1);
            } else {
                func_800CE8F0(P(D_800F32D0, 8), 0x19, &ee.a, a1);
            }
            SS(a1, 0x10) = func_80077CF4((SH(a1, 0xA) << 10) / 52) * 3 / 2;
            {
            int m = D_800E27EC;
            SH(a1, 0x12) = SH(a1, 0x10) / 48;
            if (m % 3 == 0) {
                COPYFX();
            }
            }
            if (SH(a1, 0xA) >= 0x34) {
                o = D_800F32D0;
                SH(a1, 8) = 1;
                SH(a1, 0xA) = 0;
                func_800CE9D4(P(o, 8), 0, &ee.b);
                HS(a1, 0xE) = (unsigned short)(H(a1, 0xE) + 0x400) - ee.b.y;
                if (D_800E2368[0xD] != 0) {
                    void **pp = P(D_800F32D0, 8);
                    if (pp != 0) {
                        void *r = *pp;
                        if (r != 0) {
                            unsigned char *c = P(r, 0x18);
                            if (*c == 1) *c = 2;
                        }
                    }
                }
            }
            return 0;
        case 1:
            H(a1, 0) = H(a1, 0) + func_80077DC4(SH(a1, 0xE)) * a2[0] / 4096;
            w = func_80077CF4(SH(a1, 0xE));
            t = w * a2[0];
            if (t < 0) t += 4095;
            SH(a1, 0x10) = 0x1000;
            HS(a1, 4) = H(a1, 4) + (t >> 12);
            if (D_800E27EC & 1) SH(a1, 0x12) = 0x80; else SH(a1, 0x12) = 0x60;
            if (D_800E27EC & 1) {
                COPYFX();
            }
            if (func_8001CAB0(SH(a1, 0) << 16, SH(a1, 4) << 16, D_8009D248, D_8009D1CC) == 0) {
                SH(a1, 8) = 2;
                SH(a1, 0xA) = 0;
                if (SH(a1, 0xC) != -1)
                    func_800866A4(SH(a1, 0xC), 0);
            }
            if (func_800C6B90(a1, 0x39) == 0)
                return 0;
            if (D_800E2368[0xD] != 0) {
                void *o = D_800F32D0;
                if ((**(int **)P(o, 8) & 0x3F000000) == 0x01000000) {
                    W(*D_8009D254, 0x4C) |= 0x4000;
                    **(int **)P(o, 8) = (**(int **)P(o, 8) & 0xC0FFFFFF) | 0x11000000;
                    **(int **)P(o, 8) |= 0x80000000;
                }
            }
            SH(a1, 8) = 2;
            SH(a1, 0xA) = 0;
            if (SH(a1, 0xC) != -1)
                func_800866A4(SH(a1, 0xC), 0);
            return 0;
        case 2:
            H(a1, 2)--;
            if (SH(a1, 0xA) < 0x10) {
                break;
            }
            return 1;
        }
        return 0;
    case 2:
        { short c16 = 0x10;
        __asm__("" : "=r"(c16) : "0"(c16));
        sp = (struct S *)&D_800F3368;
        { unsigned short j; *(short *)sp = c16; j = D_800E11E8; D_800F336A = 1; D_800F3376 = c16; D_800F3378 = c16; D_800F3370 = H(D_800E2850, j * 2); D_800F336C = 2; D_800F336E = 0; D_800F3372 = 0; D_800F3374 = 8; } }
        switch (SH(a1, 8)) {
        case 0:
        case 1:
            do { } while (0);
            {
                ee.b.z = D_800E27EC << 7;
                w = func_80077AA4(0x40, D_800E1208);
                func_800CEE20(a1, &ee.b, SH(a1, 0x10), SH(a1, 0x10), (&D_800F3368)[1] * 2 + 0xDC, w & 0xFFFF, 1, SH(a1, 0x12), 0);
                ASSIGN_W();
                func_800CEE20(a1, 0, SH(a1, 0x10) * 2, SH(a1, 0x10) * 2, D_800F336A * 2 + 0xD8, func_80077AA4(0x40, w) & 0xFFFF, 3, SH(a1, 0x12), 0);
                e4.x = 0x400; e4.y = 0; e4.z = 0; e4.w = 1;
                k = D_800F336C;
                e3.x = H(a1, 0); e3.z = H(a1, 4); e3.y = D_800942EC;
                w = D_800E1204[k]; if (k == 4 && D_800F3428 != 0) w += 4;
                func_800CEE20(&e3, &e4, SH(a1, 0x10), SH(a1, 0x10), D_800F336A * 2 + 0xD8, func_80077AA4(0x40, w) & 0xFFFF, 3, SH(a1, 0x12), 0);
            }
            break;
        case 2:
            {
                s1 = func_80077CF4(SH(a1, 0xA) << 6) + 0x800;
                t = func_80077DC4(SH(a1, 0xA) << 6);
                if (t < 0) t += 31;
                s2 = t >> 5;
                ASSIGN_W();
                func_800CEE20(a1, 0, s1, s1, D_800F336A + 0xFD, func_80077AA4(0x40, w) & 0xFFFF, 1, s2, 0);
                e6.x = 0x400; e6.y = 0; e6.z = 0; e6.w = 1;
                k = D_800F336C;
                e5.x = H(a1, 0); e5.z = H(a1, 4); e5.y = D_800942EC;
                w = D_800E1204[k]; if (k == 4 && D_800F3428 != 0) w += 4;
                func_800CEE20(&e5, &e6, s1, s1, D_800F336A + 0xFD, func_80077AA4(0x40, w) & 0xFFFF, 3, s2, 0);
            }
        }
        D_800F3368 = 0x10;
        D_800F336A = 1;
        D_800F3376 = 0x10;
        D_800F3378 = 0x10;
        D_800F336C = 2;
        D_800F336E = 0;
        D_800F3370 = D_800E2850[D_800E11E8];
        break;
    }
    return 0;
}
