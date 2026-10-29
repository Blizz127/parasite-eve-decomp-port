/* room_m0318i -- func_80196E4C, blob offset 0x7E64, 0x7CC bytes. NON_MATCHING
 * (behaviour C, not a byte match).
 * e21 status (in-tree harness, unmodified): MISMATCH at seed 0 on call
 * func_800D1384 a3 (calls 2 and 4, the c2 colour). Cause: retail spills &c2 to
 * sp+0x70 (`sw $t1,0x70($sp)`, retail 80196F9C, `lw $a3,0x70($sp)` at 80197090
 * / 80197154) which is 8 bytes past c2, so the harness's default 16-byte
 * stack-argument window at c2 includes a dead retail stack address (0x801FFEC8)
 * that no native frame (arena 0x801FE000-0x801FF000) can reproduce. func_800D1384
 * reads only 3 bytes through col0/col1 (pc_port/game/boot/func_800D1384_port.c
 * lines 74/76), so the compare needs STACK_EXTENTS[(0x800D1384,3)] = 3 and
 * [(0x800D1384,4)] = 3 in tools/analysis/e21_diff.py (not editable here).
 * Diagnostic only (a build with the dead slot set to the constant 0x801FFEC8,
 * NOT this file): ok, 129 seeds, retail coverage 492/499 words; the seven
 * uncovered words 80197370-80197388 are the D_800F336C == 4 arm, dead because
 * mode 2 stores 3 to D_800F336C (sh at 80197310) and only func_800CE8F0 runs
 * before the lhu at 80197350. The F layout below mirrors retail's frame.
 * Effect-object callback (a0 = mode, a1 = effect record, a2 = int *busy).
 * Mode 0 sets the record header (+0 = 6, +2 = 0x13, +4 = +6 = 0), then draws
 * four beam segments between actor joints 8 and 0x15 (func_800CE8F0 for the
 * points, func_800D1384 for the gradient segments at record +8/+0xA8/+0x58/
 * +0x118, the offsets of joints 0x15 mirrored on x) and returns the result of
 * func_800CE560 (registers the 0x10-entry task func_80196C48).
 * Mode 1 counts frames at +6; returns 2 while *a2 is nonzero, else spawns an
 * effect entry (func_800CE610) at the joint (+0 or +2 picks the joint by frame
 * parity) and returns 0. Mode 2 loads the camera matrix into the GTE, sets
 * the 0x800E27EC-dependent size, draws the glow sprite via func_800CEE20, then
 * redraws the four segments and the D_800F3368 sprite block.
 * Any other mode returns 0.
 * inline GTE/asm: Psy-Q inline_c.h-shaped GTE (COP2) SetRot/SetTrans macros. */
#define gte_SetRotMatrix(r0) __asm__ volatile ( \
    "lw $12,0(%0)\n\tlw $13,4(%0)\n\tctc2 $12,$0\n\tctc2 $13,$1\n\t" \
    "lw $12,8(%0)\n\tlw $13,12(%0)\n\tlw $14,16(%0)\n\t" \
    "ctc2 $12,$2\n\tctc2 $13,$3\n\tctc2 $14,$4" : : "r"(r0) : "$12", "$13", "$14")
#define gte_SetTransMatrix(r0) __asm__ volatile ( \
    "lw $12,20(%0)\n\tlw $13,24(%0)\n\tctc2 $12,$5\n\tlw $14,28(%0)\n\t" \
    "ctc2 $13,$6\n\tctc2 $14,$7" : : "r"(r0) : "$12", "$13", "$14")

typedef struct { short vx, vy, vz, pad; } SVECTOR;
typedef struct { unsigned char r, g, b, cd; } CVECTOR;
typedef struct { short m[3][3]; int t[3]; } MATRIX;

#define H(o, x) (*(unsigned short *)((char *)(o) + (x)))
#define SH(o, x) (*(short *)((char *)(o) + (x)))

extern SVECTOR D_8018F214;
extern SVECTOR D_8018F21C;
extern SVECTOR D_8018F224;
extern SVECTOR D_8018F22C;
extern CVECTOR D_8018F234;
extern CVECTOR D_8018F238;
extern CVECTOR D_8018F23C;
extern unsigned char *D_800F32D0;
extern unsigned char *D_800F33E0;
extern MATRIX *D_800BCFA4[];
extern int D_800E27EC;
extern unsigned short D_800E11FA;
extern unsigned short D_800E11EA;
extern unsigned short D_800E2850[];
extern unsigned short D_800E1204[];
extern int D_800F3428;
extern volatile short D_800F3368;
extern volatile short D_800F336A;
extern volatile unsigned short D_800F336C;
extern volatile short D_800F336E;
extern volatile short D_800F3370;
extern volatile short D_800F3372;
extern volatile short D_800F3374;
extern volatile short D_800F3376;
extern volatile short D_800F3378;
extern unsigned char func_80196C48[];

extern int func_80077AA4();
extern int func_800CE560();
extern short *func_800CE610();
extern void func_800CE8F0();
extern void func_800CEE20();
extern void func_800D1384();

int func_80196E4C(int mode, unsigned char *a1, int *a2)
{
    /* one frame object: retail keeps A/B, the four offsets and the three
     * colours (8-byte slots) contiguous at sp+0x28..0x70 (c2's address is saved at +0x70) and
     * func_800D1384 reads the colour block through its c0 argument. */
    struct { SVECTOR A, B, s[4]; struct { CVECTOR v; int pad; } c[3]; CVECTOR *pc2; } F;
    int s0;
    int s1;
    int w;
    int clut;
    unsigned short n;
    short *p;

    F.s[0] = D_8018F214;
    F.s[1] = D_8018F21C;
    F.s[2] = D_8018F224;
    F.s[3] = D_8018F22C;
    F.c[0].v = D_8018F234;
    F.c[1].v = D_8018F238;
    F.c[2].v = D_8018F23C;
    F.pc2 = &F.c[2].v;
    if (mode == 1) {
        n = H(a1, 6) + 1;
        H(a1, 6) = n;
        if (*a2 != 0)
            return 2;
        if ((n & 1) == 0)
            func_800CE8F0(*(void **)(D_800F32D0 + 8), SH(a1, 0), &F.s[3], &F.A);
        else
            func_800CE8F0(*(void **)(D_800F32D0 + 8), SH(a1, 2), &F.s[3], &F.A);
        p = func_800CE610(*(void **)(D_800F33E0 + 8));
        if (p == 0)
            return 0;
        p[0] = F.A.vx;
        p[1] = F.A.vy;
        H(p, 0x10) = 0;
        H(p, 0x12) = 0;
        p[2] = F.A.vz;
        return 0;
    }
    if (mode < 2) {
        if (mode != 0)
            return 0;
        SH(a1, 0) = 6;
        SH(a1, 2) = 0x13;
        SH(a1, 4) = 0;
        SH(a1, 6) = 0;
        F.s[1].vx = -F.s[1].vx;
        F.s[2].vx = -F.s[2].vx;
        func_800CE8F0(*(void **)(D_800F32D0 + 8), 8, &F.s[0], &F.A);
        func_800CE8F0(*(void **)(D_800F32D0 + 8), 8, &F.s[1], &F.B);
        func_800D1384(&F.A, &F.B, 0x3EC, &F.c[0].v, &F.c[1].v, 0x80, a1 + 8, 1);
        func_800CE8F0(*(void **)(D_800F32D0 + 8), 8, &F.s[2], &F.A);
        func_800D1384(&F.A, &F.B, 0x3EE, &F.c[2].v, &F.c[1].v, 0x80, a1 + 0xA8, 1);
        F.s[1].vx = -F.s[1].vx;
        F.s[2].vx = -F.s[2].vx;
        func_800CE8F0(*(void **)(D_800F32D0 + 8), 0x15, &F.s[0], &F.A);
        func_800CE8F0(*(void **)(D_800F32D0 + 8), 0x15, &F.s[1], &F.B);
        func_800D1384(&F.A, &F.B, 0x3EC, &F.c[0].v, &F.c[1].v, 0x80, a1 + 0x58, 1);
        func_800CE8F0(*(void **)(D_800F32D0 + 8), 0x15, &F.s[2], &F.A);
        func_800D1384(&F.A, &F.B, 0x3EE, &F.c[2].v, &F.c[1].v, 0x80, a1 + 0x118, 1);
        return func_800CE560(*(void **)(D_800F33E0 + 8), 0x14, 0x10, func_80196C48);
    }
    if (mode != 2)
        return 0;

    gte_SetRotMatrix(D_800BCFA4[0]);
    gte_SetTransMatrix(D_800BCFA4[0]);
    s1 = 0x8C;
    if (D_800E27EC & 1) {
        s0 = 0x2000;
        s1 = 0x64;
    } else {
        s0 = 0x1000;
    }
    D_800F3368 = 0x40;
    D_800F3376 = 0x40;
    D_800F3378 = 0x40;
    D_800F336A = 4;
    D_800F336C = 3;
    D_800F336E = 1;
    D_800F3372 = 0;
    D_800F3374 = 0x30;
    F.s[0].vz = 0;
    D_800F3370 = D_800E2850[D_800E11FA];
    func_800CE8F0(*(void **)(D_800F32D0 + 8), 0, &F.s[0], &F.A);
    w = D_800E1204[D_800F336C];
    if (D_800F336C == 4 && D_800F3428 != 0)
        w += 6;
    else
        w += 2;
    clut = func_80077AA4(0, w);
    func_800CEE20(&F.A, 0, (s0 * 3) >> 1, (s0 * 3) >> 1, 0, clut & 0xFFFF, 3, s1, 0);
    F.s[1].vx = -F.s[1].vx;
    F.s[2].vx = -F.s[2].vx;
    func_800CE8F0(*(void **)(D_800F32D0 + 8), 8, &F.s[0], &F.A);
    func_800CE8F0(*(void **)(D_800F32D0 + 8), 8, &F.s[1], &F.B);
    func_800D1384(&F.A, &F.B, 4, &F.c[0].v, &F.c[1].v, 0x80, a1 + 8, 1);
    func_800CE8F0(*(void **)(D_800F32D0 + 8), 8, &F.s[2], &F.A);
    func_800D1384(&F.A, &F.B, 6, &F.c[2].v, &F.c[1].v, 0x80, a1 + 0xA8, 1);
    F.s[1].vx = -F.s[1].vx;
    F.s[2].vx = -F.s[2].vx;
    func_800CE8F0(*(void **)(D_800F32D0 + 8), 0x15, &F.s[0], &F.A);
    func_800CE8F0(*(void **)(D_800F32D0 + 8), 0x15, &F.s[1], &F.B);
    func_800D1384(&F.A, &F.B, 4, &F.c[0].v, &F.c[1].v, 0x80, a1 + 0x58, 1);
    func_800CE8F0(*(void **)(D_800F32D0 + 8), 0x15, &F.s[2], &F.A);
    func_800D1384(&F.A, &F.B, 6, &F.c[2].v, &F.c[1].v, 0x80, a1 + 0x118, 1);
    D_800F3368 = 0x20;
    D_800F336A = 2;
    D_800F3376 = 0x20;
    D_800F3378 = 0x20;
    D_800F336C = 3;
    D_800F336E = 0;
    D_800F3374 = 0x10;
    D_800F3370 = D_800E2850[D_800E11EA];
    return 0;
}
