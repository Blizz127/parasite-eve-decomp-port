/* room_m0348i (PE.IMG room m0348i chunk 2, VRAM 0x8018EFE8)
 * func_80191A98 — blob offset 0x2AB0, 0x318 bytes. Profile era_o2_g0 (default).
 * M34 alligator phase 2: fire-breath beam floor-glow callback (two glow slots D_80192E80[2]).
 * Masked-body twin of room_m0174i func_80192BB0 (not yet ported).
 * inline GTE/asm: uses Psy-Q inline_c.h-shaped GTE (COP2) asm macros
 * (gte_CompMatrix = MulMatrix0 + SetTransMatrix + ldlv0/rt/stlvnl) — count separately from plain C.
 * Hand-written in this repo (khasinski's vendored tree has this body as asm only).
 * Evidence: docs/evidence/func-80191A98/REPORT.md */
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
typedef struct { int vx, vy, vz, pad; } VECTOR;
typedef struct { short vx, vy, vz, pad; } SVECTOR;
typedef struct { unsigned char b[8]; } B8;
typedef struct { char pad[0x14]; short h14; short h16; } FxSlot;

extern B8 D_8018F004;
extern FxSlot D_80192E80[2];
extern char **func_800C2B50();
extern void func_800C3238();
extern void func_800794C4();
extern void func_80071A44();
extern void func_80078CC4();
extern void func_800C4FC4();

void func_80191A98(int a0, int a1, char *a2)
{
    MATRIX m;
    MATRIX m2;
    B8 rot;
    VECTOR tmp;
    VECTOR scale;
    char **x;

    x = func_800C2B50();
    rot = D_8018F004;
    *(MATRIX *)a2 = *(MATRIX *)(*(char **)(*x + 0x238) + 0xA0);
    D_80192E80[0].h14 = *(short *)(a2 + 0x34);
    D_80192E80[1].h14 = *(short *)(a2 + 0x34);
    func_800C3238(2);
    m.m[2][2] = 0x1000; m.m[1][1] = 0x1000; m.m[0][0] = 0x1000;
    m.t[0] = m.t[1] = m.t[2] = 0;
    m.m[0][1] = m.m[0][2] = m.m[1][0] = m.m[1][2] = m.m[2][0] = m.m[2][1] = 0;
    m.t[0] = 0; m.t[1] = 100; m.t[2] = -100;
    gte_CompMatrix(a2, &m, &m);
    func_800794C4(&rot, &m);
    func_80071A44(&scale, 0, 16);
    scale.vx = *(short *)(a2 + 0x30);
    scale.vy = *(short *)(a2 + 0x30);
    scale.vz = *(short *)(a2 + 0x30);
    tmp = scale;
    func_80078CC4(&m, &tmp);
    m2 = m;
    func_800C4FC4(&D_80192E80[0], &m, 1);
    func_800C4FC4(&D_80192E80[1], &m2, 1);
}
