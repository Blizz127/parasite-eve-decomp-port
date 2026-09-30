/* func_8006DFA8 — EXE, VRAM 0x8006DFA8, file offset 0x5E7A8, 0x218 bytes.
 * 3D sound source: RotTransPers the position through the camera, derive pan from screen X and volume from the clamped depth against D_800B0CD8's near/far range.
 * Profile era_o2_g0_expand_div.
 * inline GTE/asm: uses Psy-Q inline_c.h-shaped GTE (COP2) asm macros — count separately
 * from plain C.
 * Hand-written in this repo (khasinski's vendored tree has this body as asm only).
 * Evidence: docs/evidence/func-8006DFA8/REPORT.md */
/* Psy-Q inline_c.h-shaped GTE macros (inline COP2 asm; count separately from plain C). */
#define gte_SetRotMatrix(r0) __asm__ volatile ( \
    "lw $12,0(%0)\n\tlw $13,4(%0)\n\tctc2 $12,$0\n\tctc2 $13,$1\n\t" \
    "lw $12,8(%0)\n\tlw $13,12(%0)\n\tlw $14,16(%0)\n\t" \
    "ctc2 $12,$2\n\tctc2 $13,$3\n\tctc2 $14,$4" : : "r"(r0) : "$12", "$13", "$14")
#define gte_SetTransMatrix(r0) __asm__ volatile ( \
    "lw $12,20(%0)\n\tlw $13,24(%0)\n\tctc2 $12,$5\n\tlw $14,28(%0)\n\t" \
    "ctc2 $13,$6\n\tctc2 $14,$7" : : "r"(r0) : "$12", "$13", "$14")
#define gte_ldv0(r0) __asm__ volatile ("lwc2 $0,0(%0)\n\tlwc2 $1,4(%0)" : : "r"(r0))
#define gte_ldv3(r0, r1, r2) __asm__ volatile ( \
    "lwc2 $0,0(%0)\n\tlwc2 $1,4(%0)\n\tlwc2 $2,0(%1)\n\tlwc2 $3,4(%1)\n\t" \
    "lwc2 $4,0(%2)\n\tlwc2 $5,4(%2)" : : "r"(r0), "r"(r1), "r"(r2))
#define gte_rt() __asm__ volatile ("nop\n\tnop\n\t.word 0x4A480012")
#define gte_rtps() __asm__ volatile ("nop\n\tnop\n\t.word 0x4A180001")
#define gte_rtpt() __asm__ volatile ("nop\n\tnop\n\t.word 0x4A280030")
#define gte_stlvnl(r0) __asm__ volatile ( \
    "swc2 $25,0(%0)\n\tswc2 $26,4(%0)\n\tswc2 $27,8(%0)" : : "r"(r0) : "memory")
#define gte_stsxy(r0) __asm__ volatile ("swc2 $14,0(%0)" : : "r"(r0) : "memory")
#define gte_stsxy3(r0, r1, r2) __asm__ volatile ( \
    "swc2 $12,0(%0)\n\tswc2 $13,0(%1)\n\tswc2 $14,0(%2)" : : "r"(r0), "r"(r1), "r"(r2) : "memory")
#define gte_stopz(r0) __asm__ volatile ("swc2 $24,0(%0)" : : "r"(r0) : "memory")
#define gte_stszotz(r0) __asm__ volatile ( \
    "mfc2 $12,$19\n\tnop\n\tsra $12,$12,2\n\tsw $12,0(%0)" : : "r"(r0) : "$12", "memory")
#define gte_avsz3() __asm__ volatile ("nop\n\tnop\n\t.word 0x4B58002D")
#define gte_SetGeomScreen(r0) __asm__ volatile ("ctc2 %0,$26" : : "r"(r0))

typedef struct { short vx, vy, vz, pad; } SVECTOR;
typedef struct { short m[3][3]; int t[3]; } MATRIX;
typedef struct { char pad[0xF6]; unsigned char vmin, vmax; unsigned short dnear, dfar; } SndState;

extern MATRIX *D_800BCFA4[];
extern int *D_800BCFA8;
extern unsigned char D_800B0CD8[];
extern unsigned short D_800B0DD0;
extern unsigned short D_800B0DD2;
extern void func_800661A4();
extern void func_800661CC();
extern int func_80079244();

int func_8006DFA8(SVECTOR *pos, int *pan, int *vol)
{
    int sxy;
    int p;
    int flag;
    int z;
    SndState *s;

    func_800661A4();
    s = (SndState *)D_800B0CD8;
    gte_SetRotMatrix(D_800BCFA4[0]);
    gte_SetTransMatrix(D_800BCFA4[0]);
    gte_SetGeomScreen(*D_800BCFA8);
    z = func_80079244(pos, &sxy, &p, &flag);
    p = (short)sxy;
    flag = sxy >> 16;
    func_800661CC();
    *pan = ((p + 40) << 7) / 400 + 0x40;
    if ((unsigned int)*pan >= 0x100) {
        *pan = 0xFF;
    }
    if (z < D_800B0DD0) {
        z = D_800B0DD0;
    } else if (z > D_800B0DD2) {
        z = D_800B0DD2;
    }
    z = s->dfar - z;
    z = z * z / (s->dfar - s->dnear);
    *vol = z * (s->vmax - s->vmin) / (s->dfar - s->dnear) + s->vmin;
    if ((unsigned int)*vol >= 0x80) {
        *vol = 0x7F;
    }
    return 0;
}
