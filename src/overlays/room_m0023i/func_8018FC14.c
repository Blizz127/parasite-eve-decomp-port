/* room_m0023i (PE.IMG room m0023i chunk 2, VRAM 0x8018EFE8)
 * func_8018FC14 — blob offset 0xC2C, 0xA30 bytes. Default profile era_o2_g0.
 * Eve room effect callback (same cmd 0/1/2 shape as func_8018F710).
 * cmd 0: sets +0x20 from D_800E2368->0x12, clears the counters, bumps the
 *   owner's byte flag 1 -> 2, arms +0x24 and creates the mesh D_80190760
 *   (func_8006E498 / func_800C6D5C).
 * cmd 1: func_800CE8F0/CE9D4 update, a four-state counter ramp over
 *   +0x1C/+0x2C/+0x2E, then func_800CFB7C steps +8..+0xC from the position;
 *   when func_800CEB8C passes and +0x24 is armed, it sets the owner's state
 *   bits and disarms.
 * cmd 2: two func_800CEE20 sprites and two func_800D004C rings while +0x2C
 *   is nonzero, then three func_800C6xxx / func_800C71E4 mesh passes.
 * Levers:
 *   - D_800F3368 is one struct (as in func_8018F710), and the tail writes
 *     a70 (the D_800E2850 read) before a6C/a6E. A scalar store would take
 *     gcc 2.7's MEM_IN_STRUCT alias exemption and sched2 would hoist it above
 *     the table load; a struct field store can't, so it stays after;
 *   - &eff lives in a local pinned to $a2 (register asm("$6")), so case 0
 *     reuses the value computed before the switch;
 *   - buf is 8 bytes (frame 0xC0);
 *   - D_800E2368 goes into a local before the case-0 stores;
 *   - cmd 1 state 2 builds `t = (D_800E27EC & 1) << 8` in a block-local
 *     before the counter increment and stores `t + 0x1000` after it (gives
 *     the lh reload and retail's $v0/$v1 split);
 *   - cmd 2: the first two lookups read D_800F3368.a6C directly, each with
 *     its own w; the second loads k2 before its d stores; the third uses k
 *     and w2; ip = &D_800E11EA and pm = D_800BCFA4 are pointer locals
 *     ($s1 and the la + lw pair).
 * inline GTE/asm: uses Psy-Q inline_c.h-shaped GTE (COP2) asm macros — count separately
 * from plain C.
 * Hand-written in this repo (khasinski's vendored tree has this body as asm only). */
#define gte_SetRotMatrix(r0) __asm__ volatile ( \
    "lw $12,0(%0)\n\tlw $13,4(%0)\n\tctc2 $12,$0\n\tctc2 $13,$1\n\t" \
    "lw $12,8(%0)\n\tlw $13,12(%0)\n\tlw $14,16(%0)\n\t" \
    "ctc2 $12,$2\n\tctc2 $13,$3\n\tctc2 $14,$4" : : "r"(r0) : "$12", "$13", "$14")
#define gte_SetTransMatrix(r0) __asm__ volatile ( \
    "lw $12,20(%0)\n\tlw $13,24(%0)\n\tctc2 $12,$5\n\tlw $14,28(%0)\n\t" \
    "ctc2 $13,$6\n\tctc2 $14,$7" : : "r"(r0) : "$12", "$13", "$14")

typedef struct { short x, y, z, w; } GteShortVector;
typedef struct { short m[3][3]; int t[3]; } MATRIX;
typedef struct { int vx, vy, vz, pad; } VECTOR;

#define B(o, x) (*(unsigned char *)((char *)(o) + (x)))
#define H(o, x) (*(unsigned short *)((char *)(o) + (x)))
#define SH(o, x) (*(short *)((char *)(o) + (x)))
#define W(o, x) (*(int *)((char *)(o) + (x)))
#define P(o, x) (*(void **)((char *)(o) + (x)))

extern GteShortVector D_8018EFF4;
extern unsigned char D_801906F0[];
extern int D_80190760;
extern unsigned char *D_800E2368;
extern void *D_800F32D0;
extern void **D_8009D254;
extern int D_800B0E64;
extern int D_800E27EC;
extern unsigned short D_800E11EA;
extern unsigned short D_800E2850[];
extern unsigned short D_800E1204[];
extern int D_800F3428;
extern short D_800942EC;
extern MATRIX *D_800BCFA4[];
extern struct { short a68, a6A; unsigned short a6C; short a6E; unsigned short a70; short a72, a74, a76, a78; } D_800F3368;

extern int func_8006E498();
extern void func_800C6D5C();
extern void func_800CE8F0();
extern void func_800CE9D4();
extern int func_80077DC4();
extern void func_800CFB7C();
extern int func_800CEB8C();
extern void func_800CF3AC();
extern int func_80077AA4();
extern int func_80077A64();
extern void func_800CEE20();
extern void func_800D004C();
extern void func_800794C4();
extern void func_800785D4();
extern void func_80078CC4();
extern void func_800C6EC0(unsigned short, unsigned short);
extern void func_800C6ED8();
extern void func_800C6EF8();
extern void func_800C71E4();
extern void func_800C6F4C();
extern void func_800C70EC();
extern void func_800C6FA0();

int func_8018FC14(int a0, unsigned char *a1)
{
    VECTOR sc;
    GteShortVector d;
    GteShortVector ang;
    GteShortVector eff;
    MATRIX m;
    MATRIX rot;
    unsigned char buf[8];
    int mode;
    int s4;
    int w;
    int w2;
    int k;
    int k2;
    int u;
    void *o;
    unsigned char *q;
    unsigned char *c;
    void **pp;
    void *r;
    unsigned short *ip;
    MATRIX **pm;
    register GteShortVector *e asm("$6");

    mode = a0;
    eff = D_8018EFF4;
    e = &eff;

    switch (mode) {
    case 0:
        switch (*(short *)(D_800E2368 + 0x12)) {
        case 0:
            W(a1, 0x20) = 8;
            break;
        case 1:
            W(a1, 0x20) = 0x10;
            break;
        }
        q = D_800E2368;
        SH(a1, 0x28) = 0;
        SH(a1, 0x2A) = 0;
        W(a1, 0x18) = 0;
        SH(a1, 0x2C) = 0;
        if (q[0xD] != 0) {
            pp = P(D_800F32D0, 8);
            if (pp != 0) {
                r = *pp;
                if (r != 0) {
                    c = P(r, 0x18);
                    if (*c == 1) {
                        *c = 2;
                    }
                }
            }
        }
        W(a1, 0x24) = 1;
        D_80190760 = func_8006E498(D_800B0E64, 0xC5463704, e);
        func_800C6D5C(D_80190760, 0, 0);
        return 0;
    case 1:
        func_800CE8F0(P(D_800F32D0, 8), W(a1, 0x20), e, a1);
        func_800CE9D4(P(D_800F32D0, 8), 0, a1 + 0x10);
        switch (SH(a1, 0x28)) {
        case 0:
            H(a1, 0x2A)++;
            W(a1, 0x1C) = 0x400;
            W(a1, 0x18) += 0x19A;
            SH(a1, 0x2C) = 0x1000;
            SH(a1, 0x2E) = SH(a1, 0x2A) * 8 / 5;
            if (SH(a1, 0x2A) >= 5) {
                SH(a1, 0x28) = 1;
                SH(a1, 0x2A) = 0;
            }
            break;
        case 1:
            H(a1, 0x2A)++;
            W(a1, 0x1C) = func_80077DC4(SH(a1, 0x2A) << 7) + 0x1000;
            SH(a1, 0x2C) = func_80077DC4(SH(a1, 0x2A) << 7) + 0x1000;
            SH(a1, 0x2E) = SH(a1, 0x2A) + 8;
            if (SH(a1, 0x2A) >= 8) {
                SH(a1, 0x28) = 2;
                SH(a1, 0x2A) = 0;
            }
            break;
        case 2:
            SH(a1, 0x2C) = 0x1000;
            SH(a1, 0x2E) = 0x10;
            {
                int t = (D_800E27EC & 1) << 8;
                H(a1, 0x2A)++;
                W(a1, 0x1C) = t + 0x1000;
            }
            if (SH(a1, 0x2A) >= 0x20) {
                SH(a1, 0x28) = 3;
                SH(a1, 0x2A) = 0;
            }
            break;
        case 3:
            SH(a1, 0x2C) = 0x1000;
            H(a1, 0x2A)++;
            W(a1, 0x1C) = 0x1000 - (SH(a1, 0x2A) << 8);
            SH(a1, 0x2E) = SH(a1, 0x2A) + 0x10;
            if (SH(a1, 0x2A) >= 0x10) {
                return 1;
            }
            break;
        }
        func_800CFB7C(a1 + 0x10, SH(a1, 0x18), &d);
        H(a1, 8) = H(a1, 0);
        H(a1, 0xA) = H(a1, 2);
        H(a1, 0xC) = H(a1, 4);
        H(a1, 8) += d.x;
        H(a1, 0xA) += d.y;
        H(a1, 0xC) += d.z;
        if (func_800CEB8C(a1, a1 + 8, 0x50) != 0 && W(a1, 0x24) != 0) {
            if (D_800E2368[0xD] != 0) {
                o = D_800F32D0;
                if ((**(int **)P(o, 8) & 0x3F000000) == 0x01000000) {
                    W(*D_8009D254, 0x4C) |= 0x4000;
                    **(int **)P(o, 8) = (**(int **)P(o, 8) & 0xC0FFFFFF) | 0x11000000;
                    **(int **)P(o, 8) |= 0x80000000;
                }
            }
            W(a1, 0x24) = 0;
        }
        return 0;
    case 2:
        D_800F3368.a74 = 0x18;
        D_800F3368.a68 = 0x40;
        D_800F3368.a6A = 4;
        D_800F3368.a76 = 0x40;
        D_800F3368.a78 = 0x40;
        D_800F3368.a70 = D_800E2850[D_800E11EA];
        D_800F3368.a6C = 3;
        D_800F3368.a6E = 0;
        D_800F3368.a72 = 0;
        if (SH(a1, 0x2C) != 0) {
            ang.x = 0;
            ang.y = 0;
            ang.w = 0;
            ang.z = D_800E27EC * 0x60;
            s4 = SH(a1, 0x2C);
            s4 = s4 * 2 / 3;
            if (D_800E27EC & 1) {
                s4 = s4 * 9 / 8;
            }
            func_800CF3AC(D_801906F0, buf, SH(a1, 0x2E));
            w = D_800E1204[D_800F3368.a6C];
            if (D_800F3368.a6C == 4 && D_800F3428 != 0) {
                w += 4;
            }
            func_800CEE20(a1, &ang, s4, s4, 0x80, func_80077AA4(0x30, w) & 0xFFFF, 1, 0x80, buf);
            func_800D004C(a1, 0x190, 0x190, 0xC, 0, s4, s4, buf, 0, 0x40, 1);
            func_800D004C(a1, 0xBE, 0xBE, 8, 0, s4, s4, buf, 0, 0x80, 1);
            k2 = D_800F3368.a6C;
            d.x = H(a1, 0);
            d.y = H(a1, 2);
            d.z = H(a1, 4);
            d.y = D_800942EC;
            ang.x = 0x400;
            ang.w = 1;
            w = D_800E1204[k2];
            if (k2 == 4 && D_800F3428 != 0) {
                w += 4;
            }
            func_800CEE20(&d, &ang, s4, s4, 0x80, func_80077AA4(0x30, w) & 0xFFFF, 1, 0x40, buf);
        }
        ang.x = 0;
        ang.y = 0;
        ang.z = D_800E27EC << 7;
        func_800794C4(&ang, &rot);
        m = *(MATRIX *)P(P(D_800F32D0, 8), 0x238);
        func_800785D4(&m, &rot, &m);
        sc.vx = sc.vy = W(a1, 0x1C) / 16;
        sc.vz = (W(a1, 0x18) << 12) / 8000;
        func_80078CC4(&m, &sc);
        ip = &D_800E11EA;
        m.t[0] = SH(a1, 0);
        m.t[1] = SH(a1, 2);
        m.t[2] = SH(a1, 4);
        D_800F3368.a70 = D_800E2850[*ip];
        D_800F3368.a6C = 3;
        D_800F3368.a6E = 0;
        pm = D_800BCFA4;
        gte_SetRotMatrix(pm[0]);
        gte_SetTransMatrix(pm[0]);
        u = (D_800E2850[*ip] | func_80077A64(0, 1, 0, 0)) & 0xFFFF;
        k = D_800F3368.a6C;
        w2 = D_800E1204[k];
        if (k == 4 && D_800F3428 != 0) {
            w2 += 4;
        }
        func_800C6EC0(u, func_80077AA4(0x50, w2) & 0xFFFF);
        func_800C6ED8(1);
        func_800C6EF8(D_80190760);
        func_800C71E4(D_80190760, &m);
        func_800C6F4C(D_80190760);
        func_800C6EF8(D_80190760);
        sc.vz = 0x1000;
        sc.vy = 0x1800;
        sc.vx = 0x1800;
        func_80078CC4(&m, &sc);
        func_800C70EC(D_80190760, -0xFF, -0x8C, -0xFF);
        func_800C71E4(D_80190760, &m);
        func_800C6F4C(D_80190760);
        func_800C6EF8(D_80190760);
        sc.vz = 0x1000;
        sc.vy = 0x800;
        sc.vx = 0x800;
        m.t[1] = D_800942EC;
        func_80078CC4(&m, &sc);
        func_800C6FA0(D_80190760, 0x19);
        func_800C71E4(D_80190760, &m);
        func_800C6F4C(D_80190760);
        break;
    }
    return 0;
}
