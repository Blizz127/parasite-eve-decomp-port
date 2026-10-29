/* func_800C499C — EXE, VRAM 0x800C499C, file offset 0xB519C, 0x4B4 bytes. NON_MATCHING
 * (behaviour C, proven by the e21 differential; not a byte match).
 * Untextured gouraud quad effect packet (code 0x38, 0x24 bytes) for a model
 * effect record a0 (colour bytes a0[0..0xF] scaled by the 16-bit factor at
 * a0+0x12 through func_800C608C into packet +4/+0xC/+0x14/+0x1C, OTZ offset
 * a0+0x10) with the placement matrix a1 (3x3 shorts + three-word translation).
 * Scratchpad 0x1F800000 (D_800F33B4): +0xC OTZ, +0x1C matrix, +0x30 translation.
 * a2 == 0: scratch matrix M = *D_800BCFA4 rotation times the a1 matrix
 * (three mvmva column passes), translation = *D_800BCFA4 translation +
 * R * (a1 + 0x14 vector) (mvmva with TR); GTE then loaded from the scratch matrix.
 * a2 != 0: a1 + 0x14 is transformed in place by func_800792D4 under *D_800BCFA4
 * (stack SVECTOR from the low halves of the three words, flag at sp+0x18), and
 * the GTE is loaded from the a1 matrix directly.
 * Then rtpt on the three D_800F33xx vertices (packet +8/+0x10/+0x18 XY), avsz3 ->
 * OTZ; packet code 0x38 (0x3A when D_800F337A != 0); if 0 < OTZ + a0[0x10] <= 0xFFF
 * the fourth vertex D_800F3328 is projected (rtps) to packet +0x20, the packet is
 * linked into the ordering table at D_800B0E38[D_8009CDDC] and the packet cursor
 * D_8009CDD8 advances by 0x24.
 * inline GTE/asm: Psy-Q inline_c.h-shaped GTE (COP2) asm macros — count separately
 * from plain C. */
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
#define gte_rtps() __asm__ volatile ("nop\n\tnop\n\t.word 0x4A180001")
#define gte_rtpt() __asm__ volatile ("nop\n\tnop\n\t.word 0x4A280030")
#define gte_avsz3() __asm__ volatile ("nop\n\tnop\n\t.word 0x4B58002D")
#define gte_stsxy3(r0, r1, r2) __asm__ volatile ( \
    "swc2 $12,0(%0)\n\tswc2 $13,0(%1)\n\tswc2 $14,0(%2)" : : "r"(r0), "r"(r1), "r"(r2) : "memory")
#define gte_stsxy2(r0) __asm__ volatile ("swc2 $14,0(%0)" : : "r"(r0) : "memory")
#define gte_stotz(r0) __asm__ volatile ("swc2 $7,0(%0)" : : "r"(r0) : "memory")
/* IR = R * (column src: halfwords at +0, +6, +12), stored back to column dst
 * (mvmva sf=1 mx=R v=IR cv=none lm=0). */
#define gte_rotcol(src, dst) __asm__ volatile ( \
    "lhu $12,0(%0)\n\tlhu $13,6(%0)\n\tlhu $14,12(%0)\n\t" \
    "mtc2 $12,$9\n\tmtc2 $13,$10\n\tmtc2 $14,$11\n\tnop\n\tnop\n\t.word 0x4A49E012\n\t" \
    "mfc2 $12,$9\n\tmfc2 $13,$10\n\tmfc2 $14,$11\n\t" \
    "sh $12,0(%1)\n\tsh $13,6(%1)\n\tsh $14,12(%1)" \
    : : "r"(src), "r"(dst) : "$12", "$13", "$14", "memory")
/* V0 = (lo16 of word +0 | lo16 of word +4 << 16, word +8); IR = TR + R*V0
 * (mvmva sf=1 mx=R v=V0 cv=TR lm=0); stored as three words at dst. */
#define gte_rttrans(src, dst) __asm__ volatile ( \
    "lhu $13,4(%0)\n\tlhu $12,0(%0)\n\tsll $13,$13,16\n\tor $12,$12,$13\n\t" \
    "mtc2 $12,$0\n\tlwc2 $1,8(%0)\n\tnop\n\tnop\n\t.word 0x4A480012\n\t" \
    "swc2 $9,0(%1)\n\tswc2 $10,4(%1)\n\tswc2 $11,8(%1)" \
    : : "r"(src), "r"(dst) : "$12", "$13", "memory")

typedef struct { short m[3][3]; int t[3]; } MATRIX;

#define W(o, x) (*(unsigned int *)((unsigned char *)(o) + (x)))
#define H(o, x) (*(unsigned short *)((unsigned char *)(o) + (x)))
#define SH(o, x) (*(short *)((unsigned char *)(o) + (x)))
#define B(o, x) (*(unsigned char *)((unsigned char *)(o) + (x)))

extern unsigned char *D_800F33B4;
extern int D_8009CDD8;
extern int D_8009CDDC;
extern unsigned char *D_800B0E38[];
extern unsigned char *D_800B0E58[];
extern MATRIX *D_800BCFA4;
extern unsigned char D_800F3310[];
extern unsigned char D_800F3318[];
extern unsigned char D_800F3320[];
extern unsigned char D_800F3328[];
extern unsigned char D_800F337A;

extern void func_800792D4();
extern void func_800C608C(short, unsigned char *, unsigned char *);

void func_800C499C(unsigned char *a0, unsigned char *a1, unsigned char a2)
{
    unsigned char *S;
    unsigned char *s0;
    unsigned char *s1;
    unsigned char *s2;
    unsigned int *e;
    unsigned int nv;
    unsigned int mask;
    int otz;
    short sv[4];
    int flag[2];

    s2 = a0;
    s0 = a1;
    s1 = D_800B0E58[D_8009CDDC] + D_8009CDD8;
    D_800F33B4 = (unsigned char *)0x1F800000;
    if (a2 == 0) {
        gte_SetRotMatrix(D_800BCFA4);
        S = (unsigned char *)0x1F800000;
        gte_rotcol(s0, S + 0x1C);
        S = D_800F33B4;
        gte_rotcol(s0 + 2, S + 0x1E);
        S = D_800F33B4;
        gte_rotcol(s0 + 4, S + 0x20);
        gte_SetTransMatrix(D_800BCFA4);
        gte_rttrans(s0 + 0x14, D_800F33B4 + 0x30);
        S = D_800F33B4;
        gte_SetRotMatrix(S + 0x1C);
        gte_SetTransMatrix(S + 0x1C);
    } else {
        gte_SetRotMatrix(D_800BCFA4);
        gte_SetTransMatrix(D_800BCFA4);
        sv[0] = W(s0, 0x14);
        sv[1] = W(s0, 0x18);
        sv[2] = W(s0, 0x1C);
        func_800792D4(sv, s0 + 0x14, flag);
        gte_SetRotMatrix(s0);
        gte_SetTransMatrix(s0);
    }
    func_800C608C(SH(s2, 0x12), s2, s1 + 4);
    func_800C608C(SH(s2, 0x12), s2 + 4, s1 + 0xC);
    func_800C608C(SH(s2, 0x12), s2 + 8, s1 + 0x14);
    func_800C608C(SH(s2, 0x12), s2 + 0xC, s1 + 0x1C);
    gte_ldv3(D_800F3310, D_800F3318, D_800F3320);
    gte_rtpt();
    B(s1, 3) = 8;
    B(s1, 7) = 0x38;
    gte_avsz3();
    if (D_800F337A != 0)
        B(s1, 7) = 0x3A;
    gte_stotz(D_800F33B4 + 0xC);
    S = D_800F33B4;
    W(S, 0xC) = W(S, 0xC) + SH(s2, 0x10);
    otz = W(S, 0xC);
    if ((unsigned int)(otz - 1) < 0xFFF) {
        gte_stsxy3(s1 + 8, s1 + 0x10, s1 + 0x18);
        gte_ldv0(D_800F3328);
        gte_rtps();
        gte_stsxy2(s1 + 0x20);
        mask = 0xFFFFFF;
        S = D_800F33B4;
        e = (unsigned int *)(D_800B0E38[D_8009CDDC] + (W(S, 0xC) << 2));
        W(s1, 0) = (W(s1, 0) & 0xFF000000) | (*e & mask);
        e = (unsigned int *)(D_800B0E38[D_8009CDDC] + (W(S, 0xC) << 2));
        nv = (*e & 0xFF000000) | ((unsigned int)s1 & mask);
        D_8009CDD8 += 0x24;
        *e = nv;
    }
}
