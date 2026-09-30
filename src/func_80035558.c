/* func_80035558 — EXE, VRAM 0x80035558, file offset 0x25D58, 0x72C bytes (459 words).
 * Field/battle frame pump (khasinski sym: Entity_FrameUpdate): runs every actor's +0x190
 * per-frame handler, the D_8009D1A0/D_800B0CD8 pause/event gates, camera focus
 * (func_80065E48), per-actor model matrix rebuild (RotMatrix + scale via gte_CompMatrix),
 * the +0x1B4 animation/effect ticks, clip ticks (func_8001A4AC) and actor retirement.
 * Profile era_o2_g8_force_d8009d1a0_d8009d1f4_d8009d2a4_d800bcffe_absolute (-O2 -G8; the
 * actor list D_8009D20C / D_8009D254 stay gp-relative, the four forced symbols absolute).
 * inline GTE/asm: uses Psy-Q inline_c.h-shaped GTE (COP2) asm macros (gte_CompMatrix) —
 * count separately from plain C.
 * Hand-written in this repo (khasinski's vendored tree has this body as asm only).
 * Evidence: docs/evidence/func-80035558/REPORT.md */
/* Psy-Q inline_c.h-shaped GTE macros (inline COP2 asm; count separately from plain C). */
#define gte_SetRotMatrix(r0) __asm__ volatile ( \
    "lw $12,0(%0)\n\tlw $13,4(%0)\n\tctc2 $12,$0\n\tctc2 $13,$1\n\t" \
    "lw $12,8(%0)\n\tlw $13,12(%0)\n\tlw $14,16(%0)\n\t" \
    "ctc2 $12,$2\n\tctc2 $13,$3\n\tctc2 $14,$4" : : "r"(r0) : "$12", "$13", "$14")
#define gte_SetTransMatrix(r0) __asm__ volatile ( \
    "lw $12,20(%0)\n\tlw $13,24(%0)\n\tctc2 $12,$5\n\tlw $14,28(%0)\n\t" \
    "ctc2 $13,$6\n\tctc2 $14,$7" : : "r"(r0) : "$12", "$13", "$14")
#define gte_ldclmv(r0) __asm__ volatile ( \
    "lhu $12,0(%0)\n\tlhu $13,6(%0)\n\tlhu $14,12(%0)\n\t" \
    "mtc2 $12,$9\n\tmtc2 $13,$10\n\tmtc2 $14,$11" : : "r"(r0) : "$12", "$13", "$14")
#define gte_rtir() __asm__ volatile ("nop\n\tnop\n\t.word 0x4A49E012")
#define gte_stclmv(r0) __asm__ volatile ( \
    "mfc2 $12,$9\n\tmfc2 $13,$10\n\tmfc2 $14,$11\n\t" \
    "sh $12,0(%0)\n\tsh $13,6(%0)\n\tsh $14,12(%0)" : : "r"(r0) : "$12", "$13", "$14", "memory")
#define gte_ldlv0(r0) __asm__ volatile ( \
    "lhu $13,4(%0)\n\tlhu $12,0(%0)\n\tsll $13,$13,16\n\tor $12,$12,$13\n\t" \
    "mtc2 $12,$0\n\tlwc2 $1,8(%0)" : : "r"(r0) : "$12", "$13")
#define gte_rt() __asm__ volatile ("nop\n\tnop\n\t.word 0x4A480012")
#define gte_stlvnl(r0) __asm__ volatile ( \
    "swc2 $9,0(%0)\n\tswc2 $10,4(%0)\n\tswc2 $11,8(%0)" : : "r"(r0) : "memory")
#define gte_MulMatrix0(r1, r2, r3) { \
    gte_SetRotMatrix(r1); \
    gte_ldclmv(r2); gte_rtir(); gte_stclmv(r3); \
    gte_ldclmv((char *)(r2) + 2); gte_rtir(); gte_stclmv((char *)(r3) + 2); \
    gte_ldclmv((char *)(r2) + 4); gte_rtir(); gte_stclmv((char *)(r3) + 4); }
#define gte_CompMatrix(r1, r2, r3) { \
    gte_MulMatrix0(r1, r2, r3); \
    gte_SetTransMatrix(r1); \
    gte_ldlv0((char *)(r2) + 20); gte_rt(); gte_stlvnl((char *)(r3) + 20); }

typedef struct { short m[3][3]; int t[3]; } MATRIX;
typedef struct { int vx, vy, vz; } VEC3;

extern unsigned int D_8009D1A0;
extern unsigned int D_8009D1F4;
extern short D_8009D2A4;
extern short D_800BCFFE;
extern unsigned int D_800B0CD8[];
extern char *D_8009D20C;
extern char *D_8009D254;
extern char D_800A76D8[];
extern char D_800B89F8[];

extern void func_800299CC();
extern void func_8005C174();
extern void func_80067CBC();
extern int func_8005C498();
extern void func_80069660();
extern void func_8001A9F8();
extern void func_8003601C();
extern void func_80066268();
extern void func_80065E48();
extern void func_800661A4();
extern void func_800794C4();
extern void func_8003A6A8();
extern void func_8006698C();
extern void func_80039B74();
extern void func_8003A088();
extern void func_8003AC90();
extern void func_8003AF14();
extern void func_80068014();
extern void func_8006C5BC();
extern void func_80069594();
extern void func_800661CC();
extern void func_8001A4AC();
extern void func_80036448();
extern void func_80012774();
extern void func_800360B4();

#define W(o, x) (*(int *)((char *)(o) + (x)))
#define SH(o, x) (*(short *)((char *)(o) + (x)))
#define UH(o, x) (*(unsigned short *)((char *)(o) + (x)))
#define P(o, x) (*(char **)((char *)(o) + (x)))

void func_80035558(void)
{
    VEC3 v;
    MATRIX m;
    char *a;
    char *b;
    void (*f)();
    char *c;

    if (!(D_8009D1A0 & 4)) {
        for (a = D_8009D20C; a != 0; a = P(a, 4)) {
            f = *(void (**)())(a + 0x190);
            if (f != 0) {
                f(a);
            }
        }
    }
    b = (char *)D_800B0CD8;
    if (!(*(unsigned int *)b & 0x200)) {
        if (D_8009D1A0 & 2) {
            func_800299CC();
        } else {
            if (D_8009D254 != 0 && W(D_8009D254, 0) != 0 && (D_8009D1F4 & 0x80) &&
                !(D_8009D1A0 & 0x2000) && !(*(unsigned int *)b & 0x3400)) {
                func_8005C174(0);
                func_80067CBC();
                D_8009D1A0 |= 4;
                D_800B0CD8[0] |= 0x9000;
            }
            D_8009D2A4 = func_8005C498(D_800A76D8);
            if (D_8009D2A4 != 0) {
                func_80067CBC();
                D_8009D1A0 &= ~4;
                D_800B0CD8[0] &= 0xFFFF6FFF;
            }
        }
        func_80069660();
        if (!(D_8009D1A0 & 4)) {
            func_8001A9F8();
        }
        func_8003601C();
        func_80066268();
        c = D_8009D254;
        if (c != 0) {
        found:
            v.vx = W(c, 0x28);
            v.vy = W(c, 0x2C) - (D_800BCFFE << 16);
            v.vz = W(c, 0x30);
        } else {
            v.vx = v.vy = v.vz = 0;
            for (c = D_8009D20C; c != 0; c = P(c, 4)) {
                if (W(c, 0x1AC) != 0) {
                    goto found;
                }
            }
        }
        func_80065E48(&v);
        if (!(D_8009D1A0 & 4)) {
            func_800661A4();
            for (a = D_8009D20C; a != 0; a = P(a, 4)) {
                if (!(W(a, 0x98) & 0x10040)) {
                    W(a, 0x1FC) = SH(a, 0x2A);
                    W(a, 0x200) = SH(a, 0x2E);
                    W(a, 0x204) = SH(a, 0x32);
                    UH(a, 0x1E0) = UH(a, 0x38);
                    UH(a, 0x1E2) = UH(a, 0x3A);
                    UH(a, 0x1E4) = UH(a, 0x3C);
                    func_800794C4(a + 0x1E0, a + 0x1E8);
                    m.m[0][0] = UH(a, 0x26);
                    m.m[1][1] = UH(a, 0x26);
                    m.m[2][2] = UH(a, 0x26);
                    m.t[0] = m.t[1] = m.t[2] = 0;
                    m.m[0][1] = m.m[0][2] = m.m[1][0] = m.m[1][2] = m.m[2][0] = m.m[2][1] = 0;
                    gte_CompMatrix(a + 0x1E8, &m, a + 0x1E8);
                }
            }
        }
        func_800661A4();
        for (a = D_8009D20C; a != 0; a = P(a, 4)) {
            if (a == D_8009D254 && (D_800B0CD8[0] & 0x40000)) {
                continue;
            }
            if (W(a, 0x1AC) != 0) {
                UH(P(a, 0x1B4), 0x16) = UH(a, 0x26);
            }
            if (W(a, 0x98) & 0x40) {
                if (W(a, 0x98) & 0x2000) {
                    func_8003A6A8(a + 0x1B4, D_800B89F8);
                    W(a, 0x28) = SH(a, 0x254) << 16;
                    W(a, 0x2C) = SH(a, 0x256) << 16;
                    W(a, 0x30) = SH(a, 0x258) << 16;
                } else if (W(a, 0x1AC) == 0) {
                    SH(a, 0x228) = W(a, 0x28) >> 16;
                    SH(a, 0x22A) = W(a, 0x2C) >> 16;
                    SH(a, 0x22C) = W(a, 0x30) >> 16;
                }
            } else {
                b = a + 0x1B4;
                func_8006698C(b);
                if (W(a, 0x1B0) != 0) {
                    func_80039B74(b, W(a, 0x1B0), SH(a, 0x16), 1);
                }
                func_8003A088(b);
                func_8003A6A8(b, D_800B89F8);
                if (!(W(a, 0x98) & 0x20000000)) {
                    func_8003AC90(b, D_800B89F8);
                    func_8003AF14(b, D_800B89F8);
                    if (W(a, 0x98) & 0x8800) {
                        UH(a, 0x250) |= 1;
                    }
                    func_80068014(a);
                }
            }
        }
        func_8006C5BC();
        func_80069594();
        func_800661CC();
        if (D_8009D1A0 & 4) {
            return;
        }
        if (D_8009D1A0 & 0x100) {
            if (!(D_800B0CD8[0] & 0x40000) && D_8009D254 != 0) {
                func_8001A4AC(D_8009D254);
            }
        } else {
            for (a = D_8009D20C; a != 0; a = P(a, 4)) {
                if (a == D_8009D254 && (D_800B0CD8[0] & 0x40000)) {
                    continue;
                }
                if (!(W(a, 0x98) & 0x800040)) {
                    func_8001A4AC(a);
                }
            }
        }
    }
    if (!(D_8009D1A0 & 4)) {
        func_80036448();
        func_80012774();
        func_800360B4();
        for (a = D_8009D20C; a != 0; a = P(a, 4)) {
            W(a, 0x98) &= 0xEFFFFFFF;
        }
    }
}
