/* room_m0429i — func_80192F9C, blob offset 0x3FB4, 0x2500 bytes. NON_MATCHING
 * (behaviour C, proven by the e21 differential; not a byte match).
 * Effect callback with three modes on a record a1 (frame at +0x1A, state at
 * +0x18; +0..+4 position, +8/+0xA, +0x10 sub-vectors):
 *   mode 0  binds five effect handles (func_8006E498 -> D_8019B680..B690 and
 *           func_800C6D5C), clears the state/frame, copies the record's three
 *           vectors with func_800CE870/func_800CE9D4 and registers
 *           func_80192E08 with func_800CE560 (its result is the return value).
 *   mode 1  steps the state machine (states 0..4; +0x1A counts frames, the
 *           state advances at 16/16/16/32 frames, state 4 returns 1 at 24).
 *           State 0 plays two sounds at frame 2, state 3 spawns a particle
 *           (func_800CE610 record, randomised +0/+2/+4) every frame, raises
 *           the D_8009D254 flag when the func_800C6B90 phase tests agree and
 *           sets D_8019B668 = 0x80; state 4 spawns the same particle and sets
 *           D_8019B668 = sin(f * 1024 / 24) / 32.
 *   mode 2  draws: states 0..2 are func_800CEE20 glows plus (1, 2) a
 *           func_800D004C / func_800D0728 pair (state 2 also a 16-spoke
 *           func_800D0E88 fan); states 3 and 4 are five rotated and scaled
 *           handle glows (the `glow` helper below: func_80079754 builds the
 *           rotation, func_80078CC4 scales it, func_800C6EF8/6FA0/71E4/6F4C
 *           render through the handle) plus func_800D004C / func_800D0728
 *           draws. Every path ends by resetting the D_800F3368 draw block.
 * inline GTE/asm: Psy-Q inline_c.h-shaped GTE (COP2) SetRot/SetTrans macros. */
#define gte_SetRotMatrix(r0) __asm__ volatile ( \
    "lw $12,0(%0)\n\tlw $13,4(%0)\n\tctc2 $12,$0\n\tctc2 $13,$1\n\t" \
    "lw $12,8(%0)\n\tlw $13,12(%0)\n\tlw $14,16(%0)\n\t" \
    "ctc2 $12,$2\n\tctc2 $13,$3\n\tctc2 $14,$4" : : "r"(r0) : "$12", "$13", "$14")
#define gte_SetTransMatrix(r0) __asm__ volatile ( \
    "lw $12,20(%0)\n\tlw $13,24(%0)\n\tctc2 $12,$5\n\tlw $14,28(%0)\n\t" \
    "ctc2 $13,$6\n\tctc2 $14,$7" : : "r"(r0) : "$12", "$13", "$14")

typedef struct { short x, y, z, w; } GteShortVector;
typedef struct { unsigned char c[4]; } B4;
typedef struct { short m[3][3]; int t[3]; } MATRIX;

#define H(o, x) (*(unsigned short *)((char *)(o) + (x)))
#define SH(o, x) (*(short *)((char *)(o) + (x)))
#define W(o, x) (*(int *)((char *)(o) + (x)))

extern GteShortVector D_8018F1D4;
extern B4 D_8018F1DC;
extern B4 D_8018F1E0;
extern short D_800942EC;
extern int D_800B0E64;
extern int D_800E27EC;
extern unsigned short D_800E11EA;
extern unsigned short D_800E11FA;
extern unsigned short D_800E120A;
extern unsigned short D_800E2850[];
extern unsigned short D_800E1204[];
extern int D_800F3428;
extern int D_8019B668;
extern int D_8019B680;
extern int D_8019B684;
extern int D_8019B688;
extern int D_8019B68C;
extern int D_8019B690;
extern unsigned char *D_800F32D0;
extern unsigned char *D_800F33E0;
extern unsigned char *D_800E2368;
extern int **D_8009D254;
extern MATRIX *D_800BCFA4[];
extern struct { short a68, a6A; unsigned short a6C; short a6E; unsigned short a70; short a72, a74, a76, a78; } D_800F3368;
extern unsigned char func_80192E08[];

extern int func_8006DF50();
extern int func_8006E498();
extern int func_80071A54();
extern int func_80077A64();
extern int func_80077AA4();
extern int func_80077CF4();
extern int func_80077DC4();
extern void func_80078CC4();
extern void func_80079754();
extern int func_800C6B90();
extern void func_800C6D5C();
extern void func_800C6EC0();
extern void func_800C6ED8();
extern void func_800C6EF8();
extern void func_800C6F4C();
extern void func_800C6FA0();
extern void func_800C71E4();
extern int func_800CE560();
extern unsigned char *func_800CE610();
extern void func_800CE870();
extern void func_800CE9D4();
extern void func_800CEE20();
extern void func_800D004C();
extern void func_800D0728();
extern void func_800D0E88();
extern void func_800D1AE0();
extern void func_800D1D24();
extern int func_800D3FD8();

/* One rotated and scaled handle glow: clut/colour setup, rotation from the
 * angle vector (vx, vy, vz), scale (sx, sy, sz), translation (tx, ty, tz),
 * then the four handle calls (the handle global is re-read for each). */
static void glow(GteShortVector *r, int *h, int vx, int vy, int vz, int clutbase, int sx, int sy,
                 int sz, int tx, int ty, int tz, int bright)
{
    MATRIX m;
    int sc[3];
    int v, w;

    r->x = vx;
    r->y = vy;
    r->z = vz;
    D_800F3368.a6C = 3;
    D_800F3368.a6E = 0;
    D_800F3368.a70 = D_800E2850[D_800E11EA];
    v = func_80077A64(0, 1, 0, 0);
    w = D_800E1204[D_800F3368.a6C];
    v = (D_800E2850[D_800E11EA] | v) & 0xFFFF;
    if (D_800F3368.a6C == 4 && D_800F3428 != 0)
        w += 4;
    func_800C6EC0(v, func_80077AA4(clutbase, w) & 0xFFFF);
    func_800C6ED8(1);
    func_80079754(r, &m);
    sc[0] = sx;
    sc[1] = sy;
    sc[2] = sz;
    m.t[0] = tx;
    m.t[1] = ty;
    m.t[2] = tz;
    func_80078CC4(&m, sc);
    func_800C6EF8(*h);
    func_800C6FA0(*h, bright & 0xFFFF);
    func_800C71E4(*h, &m);
    func_800C6F4C(*h);
}

/* The randomised particle record spawned by mode 1 states 3 and 4. */
static void spawn(unsigned char *a1)
{
    unsigned char *s0;

    s0 = func_800CE610(W(D_800F33E0, 8));
    if (s0 != 0) {
        H(s0, 0) = H(a1, 0);
        H(s0, 2) = H(a1, 2);
        H(s0, 4) = H(a1, 4);
        H(s0, 2) = H(s0, 2) - func_80071A54() % 800;
        H(s0, 0) = H(s0, 0) + (func_80071A54() % 512 - 0x100);
        {
            int r = func_80071A54() % 512;
            H(s0, 8) = 0;
            H(s0, 0xA) = 0;
            H(s0, 4) = H(s0, 4) + (r - 0x100);
        }
    }
}

int func_80192F9C(int mode, unsigned char *a1)
{
    /* retail's contiguous frame at sp+0x30: record position, the fan vector,
     * {0x400, 0, 0, 1}, the glow angle vector and two colours.  Callees read
     * a 16-byte window through these pointers, so the layout is kept. */
    struct { GteShortVector p, u, v40, vec; B4 colB, padB, colC, padC; } F;
    GteShortVector g, g2;
    int f, s1, s2, s3, s4, s6, t, i;
    int *q;

    F.v40 = D_8018F1D4;
    F.colB = D_8018F1DC;
    F.colC = D_8018F1E0;
    switch (mode) {
    case 0:
        t = func_8006E498(D_800B0E64, 0xC54C0704);
        D_8019B680 = t;
        func_800C6D5C(t, 0, 0);
        t = func_8006E498(D_800B0E64, 0xC58C0704);
        D_8019B684 = t;
        func_800C6D5C(t, 0, 0);
        t = func_8006E498(D_800B0E64, 0xC5CC0704);
        D_8019B688 = t;
        func_800C6D5C(t, 0, 0);
        t = func_8006E498(D_800B0E64, 0xC60C0704);
        D_8019B68C = t;
        func_800C6D5C(t, 0, 0);
        t = func_8006E498(D_800B0E64, 0xC64C0704);
        D_8019B690 = t;
        func_800C6D5C(t, 0, 0);
        SH(a1, 0x18) = 0;
        SH(a1, 0x1A) = 0;
        func_800CE870(W(D_800F32D0, 8), 1, a1);
        H(a1, 2) = D_800942EC;
        func_800CE870(W(D_800F32D0, 8), 0, a1 + 8);
        func_800CE9D4(W(D_800F32D0, 8), 0, a1 + 0x10);
        return func_800CE560(W(D_800F33E0, 8), 0xC, 8, func_80192E08);
    case 1:
        switch (SH(a1, 0x18)) {
        case 0:
            H(a1, 0x1A)++;
            if (SH(a1, 0x1A) == 2 && D_800B0E64 != 0) {
                t = func_800D3FD8();
                func_8006DF50(D_800B0E64, 0x5E3, t, 0x80, 0x7F);
                if (D_800B0E64 != 0)
                    func_8006DF50(D_800B0E64, 0x5E4, 0x80, 0x80, 0x7F);
            }
            if (SH(a1, 0x1A) < 0x10)
                return 0;
            SH(a1, 0x18) = 1;
            SH(a1, 0x1A) = 0;
            return 0;
        case 1:
            H(a1, 0x1A)++;
            if (SH(a1, 0x1A) < 0x10)
                return 0;
            SH(a1, 0x18) = 2;
            SH(a1, 0x1A) = 0;
            return 0;
        case 2:
            H(a1, 0x1A)++;
            if (SH(a1, 0x1A) < 0x10)
                return 0;
            SH(a1, 0x18) = 3;
            SH(a1, 0x1A) = 0;
            return 0;
        case 3:
            H(a1, 0x1A)++;
            if (func_800C6B90(a1, 0x960) != 0 &&
                (func_800C6B90(a1, 0x4B0) != 0 || func_800C6B90(a1, 0x6A4) == 0)) {
                if (D_800E2368[0xD] != 0 &&
                    (**(int **)(W(D_800F32D0, 8)) & 0x3F000000) == 0x01000000) {
                    W(*D_8009D254, 0x4C) |= 0x4000;
                    q = *(int **)(W(D_800F32D0, 8));
                    *q = (*q & 0xC0FFFFFF) | 0x2D000000;
                    q = *(int **)(W(D_800F32D0, 8));
                    *q |= 0x80000000;
                }
            }
            if (SH(a1, 0x1A) < 0x11)
                func_800D1D24(2, 0x10, SH(a1, 0x1A));
            spawn(a1);
            D_8019B668 = 0x80;
            if (SH(a1, 0x1A) < 0x20)
                return 0;
            SH(a1, 0x18) = 4;
            SH(a1, 0x1A) = 0;
            return 0;
        case 4:
            H(a1, 0x1A)++;
            spawn(a1);
            t = func_80077DC4((SH(a1, 0x1A) << 10) / 24);
            D_8019B668 = t / 32;
            if (SH(a1, 0x1A) < 0x18)
                return 0;
            SH(a1, 0x18) = 4;
            SH(a1, 0x1A) = 0;
            return 1;
        }
        return 0;
    case 2:
        F.p.x = H(a1, 0);
        F.p.y = H(a1, 2);
        F.p.z = H(a1, 4);
        gte_SetRotMatrix(D_800BCFA4[0]);
        gte_SetTransMatrix(D_800BCFA4[0]);
        D_800F3368.a72 = 0;
        D_800F3368.a74 = 4;
        switch (SH(a1, 0x18)) {
        case 0:
            s3 = func_80077CF4(SH(a1, 0x1A) << 6) / 32;
            D_800F3368.a68 = 0x40;
            D_800F3368.a6A = 4;
            D_800F3368.a76 = 0x40;
            D_800F3368.a78 = 0x40;
            D_800F3368.a6C = 3;
            D_800F3368.a6E = 1;
            D_800F3368.a70 = D_800E2850[D_800E11FA];
            if (D_800E27EC & 1)
                s3 = s3 * 2 / 3;
            g.x = F.p.x;
            g.y = D_800942EC;
            g.z = F.p.z;
            g2.x = 0x400;
            g2.y = 0;
            g2.z = 0;
            g2.w = 1;
            t = func_80077AA4(0, D_800E120A + 2);
            func_800CEE20(&g, &g2, 0x2000, 0x2000, 0, t & 0xFFFF, 1, s3, 0);
            break;
        case 1:
            s3 = 0x80;
            D_800F3368.a6A = 4;
            D_800F3368.a68 = 0x40;
            D_800F3368.a76 = 0x40;
            D_800F3368.a78 = 0x40;
            D_800F3368.a6C = 3;
            D_800F3368.a6E = 1;
            D_800F3368.a70 = D_800E2850[D_800E11FA];
            s6 = SH(a1, 0x1A) << 6;
            if (D_800E27EC & 1)
                s3 = 0x55;
            g.x = F.p.x;
            g.y = D_800942EC;
            g.z = F.p.z;
            g2.x = 0x400;
            g2.y = 0;
            g2.z = 0;
            g2.w = 1;
            t = func_80077AA4(0, D_800E120A + 2);
            func_800CEE20(&g, &g2, 0x2000, 0x2000, 0, t & 0xFFFF, 1, s3, 0);
            s3 = func_80077CF4(s6) / 32;
            if (D_800E27EC & 1)
                s3 = s3 * 2 / 3;
            s4 = func_80077CF4(s6);
            if (D_800E27EC & 1)
                s4 = s4 * 15 / 16;
            glow(&F.vec, &D_8019B684, -0x400, 0, D_800E27EC << 5, 0x20, s4 / 2, s4 / 2,
                 (SH(a1, 0x1A) << 6) + 0x400, F.p.x, F.p.y, F.p.z, 0x40);
            gte_SetRotMatrix(D_800BCFA4[0]);
            gte_SetTransMatrix(D_800BCFA4[0]);
            func_800D004C(&F.p, 0x12C, 0x12C, 0xC, 0, 0x1000, 0x1000, &F.colB, 0, s3, 1);
            s3 = func_80077DC4(s6) / 32;
            s4 = func_80077CF4(s6);
            func_800D0728(&F.p, 0x7D0, 0xB54, 0x18, &F.v40, s4, s4, 0, &F.colB, s3, 1);
            break;
        case 2:
            s3 = 0x80;
            D_800F3368.a68 = 0x40;
            D_800F3368.a6A = 4;
            D_800F3368.a76 = 0x40;
            D_800F3368.a78 = 0x40;
            D_800F3368.a70 = D_800E2850[D_800E11FA];
            D_800F3368.a6C = 3;
            D_800F3368.a6E = 1;
            s6 = SH(a1, 0x1A) << 6;
            if (D_800E27EC & 1)
                s3 = 0x55;
            s4 = 0x1000;
            g.x = F.p.x;
            g.y = D_800942EC;
            g.z = F.p.z;
            g2.x = 0x400;
            g2.y = 0;
            g2.z = 0;
            g2.w = 1;
            t = func_80077AA4(0, D_800E120A + 2);
            func_800CEE20(&g, &g2, 0x2000, 0x2000, 0, t & 0xFFFF, 1, s3, 0);
            if (D_800E27EC & 1)
                s4 = 0xF00;
            glow(&F.vec, &D_8019B684, -0x400, 0, D_800E27EC << 5, 0x20, s4 / 2, s4 / 2,
                 (SH(a1, 0x1A) << 6) + 0x800, F.p.x, F.p.y, F.p.z, 0x40);
            gte_SetRotMatrix(D_800BCFA4[0]);
            gte_SetTransMatrix(D_800BCFA4[0]);
            func_800D004C(&F.p, 0x12C, 0x12C, 0xC, 0, 0x1000, 0x1000, &F.colB, 0, s3, 1);
            s3 = 0x80;
            if (D_800E27EC & 1)
                s3 = 0x64;
            s4 = func_80077DC4(s6) / 8;
            s2 = func_80077CF4(s6) * 2 / 3;
            for (i = 0; i < 0x10; i++) {
                int a;

                a = (i << 8) + D_800E27EC * 4;
                F.u.x = F.p.x;
                F.u.y = F.p.y;
                F.u.z = F.p.z;
                F.u.x += func_80077DC4(a) * 0x7D0 / 0x1000;
                F.u.z += func_80077CF4(a) * 0x7D0 / 0x1000;
                F.vec.x = 0x400;
                F.vec.y = 0;
                F.vec.z = a + 0x400;
                func_800D0E88(&F.u, &F.vec, s2, s4, &F.colB, 0, 0, (short)s3, 1);
            }
            s3 = func_80077DC4(s6) / 32;
            s4 = func_80077DC4(s6);
            F.vec.x = H(a1, 0x10) - 0x200;
            F.vec.y = H(a1, 0x12) + 0x400;
            F.vec.z = H(a1, 0x14);
            func_800D0728(a1 + 8, 0x7D0, 0xA8C, 0x20, &F.vec, s4, s4, &F.colC, 0, s3, 1);
            F.vec.x += 0x400;
            func_800D0728(a1 + 8, 0x7D0, 0xA8C, 0x20, &F.vec, s4, s4, &F.colC, 0, s3, 1);
            break;
        case 3:
            f = SH(a1, 0x1A);
            s6 = f << 5;
            if (f == 0)
                func_800D1AE0(&F.colB, 0x46, 2, 8);
            else if (f < 9)
                func_800D1AE0(&F.colB, 0x80 - (f << 4), 1, 8);
            s4 = func_80077DC4(s6) / 2 + 0x400;
            s2 = func_80077CF4(s6) / 6 + 0x555;
            s3 = 0x80;
            if (D_800E27EC & 1)
                s3 = 0x78;
            glow(&F.vec, &D_8019B680, 0, D_800E27EC << 5, 0, 0x20, s4, s2, s4, F.p.x, F.p.y, F.p.z, s3);
            s4 = func_80077CF4(s6) / 3 + 0x400;
            s2 = func_80077DC4(s6) / 2;
            s3 = func_80077DC4(s6) / 32;
            if (D_800E27EC & 1)
                s3 = s3 * 15 / 16;
            glow(&F.vec, &D_8019B688, 0, D_800E27EC * -0x30, 0, 0x20, s4, s2, s4, F.p.x, F.p.y, F.p.z, s3 / 2);
            s1 = (SH(a1, 0x1A) << 10) / 56;
            s4 = func_80077CF4(s1) / 4 + 0xC00;
            s2 = func_80077DC4(s1) / 4 + 0x400;
            s3 = func_80077DC4(s1) / 32;
            if (D_800E27EC & 1)
                s3 = s3 * 15 / 16;
            glow(&F.vec, &D_8019B68C, 0, D_800E27EC * -0x20, 0, 0x20, s4, s2, s4, F.p.x,
                 (short)(F.p.y - SH(a1, 0x1A) * 0x18), F.p.z, s3 / 2);
            gte_SetRotMatrix(D_800BCFA4[0]);
            gte_SetTransMatrix(D_800BCFA4[0]);
            func_800D004C(&F.p, 0x9C4, 0x9C4, 0xC, 0, 0x1000, 0x1000, &F.colB, 0, s3, 1);
            s4 = func_80077CF4(s6) / 4 + 0xC00;
            s3 = func_80077DC4(s6) / 32;
            func_800D0728(&F.p, 0x76C, 0xA28, 0x18, &F.v40, s4, s4, &F.colC, 0, s3, 1);
            F.p.y -= 0x400;
            s4 = func_80077CF4(s6) * 3 / 2 + 0x1000;
            func_800D0728(&F.p, 0x3E8, 0x5DC, 0x18, &F.v40, s4, s4, &F.colB, 0, s3, 1);
            F.p.y += 0x400;
            s4 = func_80077CF4(s1) / 4 + 0x1200;
            s2 = func_80077DC4(s1);
            s3 = func_80077DC4(s1) / 32;
            if (D_800E27EC & 1)
                s3 = s3 * 15 / 16;
            glow(&F.vec, &D_8019B690, 0, (D_800E27EC << 5) + 0x400, 0, 0x60, s4, s2, s4, F.p.x, F.p.y, F.p.z, s3);
            s4 = func_80077CF4(s1) / 6 + 0x1200;
            s2 = func_80077DC4(s1) * 2;
            s3 = func_80077DC4(s1) / 32;
            if (!(D_800E27EC & 1))
                s3 = s3 * 15 / 16;
            glow(&F.vec, &D_8019B690, 0, D_800E27EC << 5, 0, 0x60, s4, s2, s4, F.p.x, F.p.y, F.p.z, s3);
            break;
        case 4:
            f = SH(a1, 0x1A);
            s6 = (f << 10) / 24;
            s4 = (0x400 - s6) / 2 + 0x200;
            s2 = func_80077CF4(s6) / 6 + 0x6AA;
            s3 = func_80077DC4(s6) / 32;
            if (D_800E27EC & 1)
                s3 = s3 * 15 / 16;
            glow(&F.vec, &D_8019B680, 0, D_800E27EC << 5, 0, 0x20, s4, s2, s4, F.p.x, F.p.y, F.p.z, s3);
            s1 = ((SH(a1, 0x1A) + 0x20) << 10) / 56;
            s4 = func_80077CF4(s1) / 4 + 0xC00;
            s2 = func_80077DC4(s1) / 4 + 0x400;
            s3 = func_80077DC4(s1) / 32;
            if (D_800E27EC & 1)
                s3 = s3 * 15 / 16;
            glow(&F.vec, &D_8019B68C, 0, D_800E27EC * -0x20, 0, 0x20, s4, s2, s4, F.p.x,
                 (short)(F.p.y - (SH(a1, 0x1A) + 0x20) * 0x18), F.p.z, s3 / 2);
            s4 = func_80077CF4(s1) / 4 + 0x1200;
            s2 = func_80077DC4(s1);
            s3 = func_80077DC4(s1) / 32;
            if (D_800E27EC & 1)
                s3 = s3 * 15 / 16;
            glow(&F.vec, &D_8019B690, 0, (D_800E27EC << 5) + 0x400, 0, 0x60, s4, s2, s4, F.p.x, F.p.y, F.p.z, s3);
            s4 = func_80077CF4(s1) / 6 + 0x1200;
            s2 = func_80077DC4(s1) * 2;
            s3 = func_80077DC4(s1) / 32;
            if (!(D_800E27EC & 1))
                s3 = s3 * 15 / 16;
            glow(&F.vec, &D_8019B690, 0, D_800E27EC << 5, 0, 0x60, s4, s2, s4, F.p.x, F.p.y, F.p.z, s3);
            break;
        default:
            break;
        }
        D_800F3368.a6A = 1;
        D_800F3368.a68 = 0x10;
        D_800F3368.a76 = 0x10;
        D_800F3368.a78 = 0x10;
        D_800F3368.a76 = 0x80;
        D_800F3368.a78 = 0x10;
        D_800F3368.a6C = 3;
        D_800F3368.a6E = 0;
        D_800F3368.a72 = 0;
        D_800F3368.a70 = D_800E2850[D_800E11EA];
        return 0;
    }
    return 0;
}
