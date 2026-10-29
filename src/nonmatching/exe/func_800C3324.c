/* func_800C3324 — EXE, VRAM 0x800C3324, file offset 0xB3B24, 0x7E0 bytes. NON_MATCHING
 * (behaviour C, proven by the e21 differential; not a byte match).
 * Textured/shaded quad effect packet (code 0x2C, 0x28 bytes) for a model
 * effect record a0: builds the packet colour (a0[0x20..0x22], optionally scaled by
 * the 16-bit factor at a0+0x28 unless it is 0x80, clamped to 0x7FFF, >> 7), a scratchpad
 * matrix pair (0x1F80001C = copy of *D_800BCFA4, translation advanced by the
 * a0 offset vector through the rotation rows; 0x1F80003C = matrix built by
 * func_80079754/func_80078CC4 from the a0+8 angles and a0+0x10 scale), the
 * concatenation M1 = M1 * M2 and translation M1.t = clamp16(M1.t + R*M2.t) via
 * two GTE mvmva passes, an RTPT of the three D_800F33xx vertices (corner screen
 * coordinates -> packet +8/+0x10/+0x18, OTZ -> scratch +0xC, UV offsets from
 * the scratch bytes +0x10/+0x11 and D_800F345C/D), an RTPS of D_800F3328 ->
 * packet +0x20, the func_80077AA4 CLUT/tpage word -> packet +0xE, and links the
 * packet into the ordering table at (OTZ + a0->0x26).
 * Scratchpad layout used: +0x10..+0x13 bytes, +0x14 word, +0xC OTZ,
 * +0x1C M1 (rot 0x1C..0x2D, trans 0x30..0x38), +0x3C M2 (rot, trans +0x50).
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

typedef struct { char *ordering[2]; char *other_buffers[6]; char *packets[2]; } RenderBufferPrefix;

extern unsigned char *D_800F33B4;
extern int D_8009CDD8;
extern int D_8009CDDC;
extern RenderBufferPrefix D_800B0E38;
extern MATRIX *D_800BCFA4;
extern unsigned char D_800F3310[];
extern unsigned char D_800F3318[];
extern unsigned char D_800F3320[];
extern unsigned char D_800F3328[];
extern unsigned char D_800F337A;
extern unsigned char D_800F345C;
extern unsigned char D_800F345D;
extern unsigned short D_800E27AC;
extern unsigned short D_800F341C;
extern unsigned short D_800F341E;

extern void func_80079754();
extern void func_80078CC4();
extern int func_80077AA4();

void func_800C3324(unsigned char *a)
{
    unsigned char *S;
    unsigned char *s0;
    unsigned int v;
    unsigned int *src;
    unsigned int *e;
    unsigned int nv;
    int i;
    int d;
    int lo;

    S = (unsigned char *)0x1F800000;
    D_800F33B4 = S;
    B(S, 0x11) = a[0x24] & 0xF0;
    B(S, 0x10) = (a[0x24] - (a[0x24] & 0xF0)) << 4;
    B(S, 0x12) = a[0x25] << 4;
    s0 = (unsigned char *)(D_800B0E38.packets[D_8009CDDC] + D_8009CDD8);
    B(S, 0x13) = a[0x25] >> 4;
    if (H(a, 0x28) != 0x80) {
        W(S, 0x14) = a[0x20];
        v = a[0x20] * H(a, 0x28);
        W(S, 0x14) = v;
        if (v > 0x7FFF)
            W(S, 0x14) = 0x7FFF;
        v = W(S, 0x14) >> 7;
        W(S, 0x14) = v;
        B(s0, 4) = v;
        W(D_800F33B4, 0x14) = a[0x21];
        v = a[0x21] * H(a, 0x28);
        W(D_800F33B4, 0x14) = v;
        if (v > 0x7FFF)
            W(D_800F33B4, 0x14) = 0x7FFF;
        v = W(D_800F33B4, 0x14) >> 7;
        W(D_800F33B4, 0x14) = v;
        B(s0, 5) = v;
        W(D_800F33B4, 0x14) = a[0x22];
        v = a[0x22] * H(a, 0x28);
        W(D_800F33B4, 0x14) = v;
        if (v > 0x7FFF)
            W(D_800F33B4, 0x14) = 0x7FFF;
        v = W(D_800F33B4, 0x14) >> 7;
        W(D_800F33B4, 0x14) = v;
    } else {
        B(s0, 4) = a[0x20];
        B(s0, 5) = a[0x21];
        v = a[0x22];
    }
    B(s0, 6) = v;
    func_80079754(a + 8, D_800F33B4 + 0x3C);
    func_80078CC4(D_800F33B4 + 0x3C, a + 0x10);
    S = D_800F33B4;
    W(S, 0x50) = 0;
    W(S, 0x54) = 0;
    W(S, 0x58) = 0;
    src = (unsigned int *)D_800BCFA4;
    for (i = 0; i < 8; i++)
        W(S, 0x1C + 4 * i) = src[i];

    S = D_800F33B4;
    d = SH(S, 0x20) * SH(a, 4) + SH(S, 0x1E) * SH(a, 2) + SH(S, 0x1C) * SH(a, 0);
    W(S, 0x30) = W(S, 0x30) + d / 4096;
    d = SH(S, 0x26) * SH(a, 4) + SH(S, 0x24) * SH(a, 2) + SH(S, 0x22) * SH(a, 0);
    W(S, 0x34) = W(S, 0x34) + d / 4096;
    d = SH(S, 0x2C) * SH(a, 4) + SH(S, 0x2A) * SH(a, 2) + SH(S, 0x28) * SH(a, 0);
    W(S, 0x38) = W(S, 0x38) + d / 4096;

    gte_SetRotMatrix(S + 0x1C);
    gte_rotcol(S + 0x3C, S + 0x1C);
    S = D_800F33B4;
    gte_rotcol(S + 0x3E, S + 0x1E);
    S = D_800F33B4;
    gte_rotcol(S + 0x40, S + 0x20);
    S = D_800F33B4;
    gte_SetTransMatrix(S + 0x1C);
    gte_rttrans(S + 0x50, S + 0x30);
    S = D_800F33B4;
    gte_SetRotMatrix(S + 0x1C);
    gte_SetTransMatrix(S + 0x1C);
    gte_ldv3(D_800F3310, D_800F3318, D_800F3320);
    gte_rtpt();
    S = D_800F33B4;
    B(s0, 3) = 9;
    B(s0, 7) = 0x2C;
    B(s0, 0xC) = B(S, 0x10);
    S = D_800F33B4;
    B(s0, 0xD) = B(S, 0x11);
    S = D_800F33B4;
    B(s0, 0x14) = B(S, 0x10) + D_800F345C;
    S = D_800F33B4;
    B(s0, 0x15) = B(S, 0x11);
    S = D_800F33B4;
    B(s0, 0x1C) = B(S, 0x10);
    S = D_800F33B4;
    B(s0, 0x1D) = B(S, 0x11) + D_800F345D;
    S = D_800F33B4;
    B(s0, 0x24) = B(S, 0x10) + D_800F345C;
    S = D_800F33B4;
    B(s0, 0x25) = B(S, 0x11) + D_800F345D;
    gte_avsz3();
    if (D_800F337A != 0)
        B(s0, 7) = B(s0, 7) | 2;
    else
        B(s0, 7) = B(s0, 7) & 0xFD;
    gte_stotz(D_800F33B4 + 0xC);
    gte_stsxy3(s0 + 8, s0 + 0x10, s0 + 0x18);
    gte_ldv0(D_800F3328);
    gte_rtps();
    H(s0, 0x16) = D_800E27AC;
    S = D_800F33B4;
    H(s0, 0xE) = func_80077AA4(D_800F341C + B(S, 0x12), D_800F341E + B(S, 0x13));
    gte_stsxy2(s0 + 0x20);
    lo = SH(a, 0x26);
    S = D_800F33B4;
    e = (unsigned int *)(D_800B0E38.ordering[D_8009CDDC] + ((W(S, 0xC) + lo) << 2));
    W(s0, 0) = (W(s0, 0) & 0xFF000000) | (*e & 0xFFFFFF);
    S = D_800F33B4;
    e = (unsigned int *)(D_800B0E38.ordering[D_8009CDDC] + ((W(S, 0xC) + SH(a, 0x26)) << 2));
    nv = (*e & 0xFF000000) | ((unsigned int)s0 & 0xFFFFFF);
    D_8009CDD8 += 0x28;
    *e = nv;
}
