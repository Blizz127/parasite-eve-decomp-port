/* func_800C9398 -- EXE, VRAM 0x800C9398, file offset 0xB9B98, 0x4D0 bytes. NON_MATCHING
 * (behaviour C, proven by the e21 differential; not a byte match).
 * Model-effect draw with two sub-parts for the effect record a2: sets up the
 * effect environment (func_800C2EAC(3), func_800C3098(0x10), func_800C2FF0(0x20,
 * 0x10), func_800C3238(2)), stores a2->h4 into D_800F34B2, then twice:
 *   - RotMatrix (func_80079754) of an 8-byte angle pair (first pass the 8 bytes
 *     at D_800C2184, second pass the 8 bytes at D_800C218C) -> 3x3 matrix R2,
 *   - copy of the 32-byte matrix a2+0x10 into a local MATRIX M and its
 *     translation replaced by the halfwords a2+8/+0xA/+0xC,
 *   - M.m = M.m * R2 (three GTE mvmva column passes) and M.t = clamp16(TR + R*0)
 *     (mvmva with a zero vector; the translation is the clamped original),
 *   - func_80078CC4(&M, D_800C2194[0..3]) scale, func_800C42A4(D_800F34A8, &M, 0).
 * The second pass uses the second angle pair and does not re-zero the vector.
 * inline GTE/asm: Psy-Q inline_c.h-shaped GTE (COP2) asm macros. */
#define gte_SetRotMatrix(r0) __asm__ volatile ( \
    "lw $12,0(%0)\n\tlw $13,4(%0)\n\tctc2 $12,$0\n\tctc2 $13,$1\n\t" \
    "lw $12,8(%0)\n\tlw $13,12(%0)\n\tlw $14,16(%0)\n\t" \
    "ctc2 $12,$2\n\tctc2 $13,$3\n\tctc2 $14,$4" : : "r"(r0) : "$12", "$13", "$14")
#define gte_SetTransMatrix(r0) __asm__ volatile ( \
    "lw $12,20(%0)\n\tlw $13,24(%0)\n\tctc2 $12,$5\n\tlw $14,28(%0)\n\t" \
    "ctc2 $13,$6\n\tctc2 $14,$7" : : "r"(r0) : "$12", "$13", "$14")
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

typedef struct { short m[3][3]; short pad; int t[3]; } MATRIX;
typedef struct { unsigned char c[8]; } B8;
typedef struct { unsigned char c[16]; } B16;

#define SH(o, x) (*(short *)((unsigned char *)(o) + (x)))
#define H(o, x) (*(unsigned short *)((unsigned char *)(o) + (x)))

extern B8 D_800C2184;
extern B8 D_800C218C;
extern B16 D_800C2194;
extern unsigned short D_800F34B2;
extern unsigned char D_800F34A8[];

extern void func_800C2EAC(int a0);
extern void func_800C3098(int a0);
extern void func_800C2FF0(int a0, int a1);
extern void func_800C3238(int a0);
extern void func_80079754();
extern void func_80078CC4();
extern void func_800C42A4();

void func_800C9398(int a0, int a1, unsigned char *a2)
{
    B8 ang[2];
    short r2[3][3];
    int vz[3];
    B16 sc;
    B16 sc2;
    MATRIX m;
    int *tv;
    int pass;

    ang[0] = D_800C2184;
    ang[1] = D_800C218C;
    func_800C2EAC(3);
    func_800C3098(0x10);
    func_800C2FF0(0x20, 0x10);
    func_800C3238(2);
    D_800F34B2 = H(a2, 4);
    func_80079754(&ang[0], r2);
    m = *(MATRIX *)(a2 + 0x10);
    m.t[0] = SH(a2, 8);
    m.t[1] = SH(a2, 0xA);
    m.t[2] = SH(a2, 0xC);
    vz[0] = 0;
    vz[1] = 0;
    vz[2] = 0;
    gte_SetRotMatrix(&m);
    gte_rotcol(&r2[0][0], &m.m[0][0]);
    gte_rotcol(&r2[0][1], &m.m[0][1]);
    gte_rotcol(&r2[0][2], &m.m[0][2]);
    gte_SetTransMatrix(&m);
    gte_rttrans(vz, m.t);
    sc = D_800C2194;
    func_80078CC4(&m, &sc);
    func_800C42A4(D_800F34A8, &m, 0);

    func_80079754(&ang[1], r2);
    m = *(MATRIX *)(a2 + 0x10);
    m.t[0] = SH(a2, 8);
    m.t[1] = SH(a2, 0xA);
    m.t[2] = SH(a2, 0xC);
    gte_SetRotMatrix(&m);
    gte_rotcol(&r2[0][0], &m.m[0][0]);
    gte_rotcol(&r2[0][1], &m.m[0][1]);
    gte_rotcol(&r2[0][2], &m.m[0][2]);
    gte_SetTransMatrix(&m);
    gte_rttrans(vz, m.t);
    sc2 = D_800C2194;
    func_80078CC4(&m, &sc2);
    func_800C42A4(D_800F34A8, &m, 0);
}
