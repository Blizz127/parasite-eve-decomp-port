/* func_800C689C -- EXE, VRAM 0x800C689C, file offset 0xB709C, 0x284 bytes.
 * Three GTE OP (cross product) calls on scratchpad (0x1F800000) edge/delta
 * vectors built from a0 (six halfwords at +0,+4,+8,+0xC,+0x10,+0x14) and three
 * halfwords of *D_8009D254 (+0x2A,+0x2E,+0x32). Returns nonzero when the three
 * cross-product Z words (0x1F800064/74/84) are not all equal in the
 * (x^y)|(y^z) > 0 sense.
 * NON_MATCHING behaviour C. inline GTE/asm: Psy-Q-shaped COP2 asm macro. */
/* rotation rows from r0[0..2] (ctc2 0,2,4), vector from v0[0..2] (IR1..IR3), OP, MAC1..3 -> o0[0..2] */
#define gte_op_rowvec(r0, v0, o0) __asm__ volatile ( \
    "lw $12,0(%0)\n\tlw $13,4(%0)\n\tctc2 $12,$0\n\tlw $14,8(%0)\n\t" \
    "ctc2 $13,$2\n\tctc2 $14,$4\n\t" \
    "lwc2 $11,8(%1)\n\tlwc2 $9,0(%1)\n\tlwc2 $10,4(%1)\n\tnop\n\tnop\n\t" \
    ".word 0x4B70000C\n\t" \
    "swc2 $25,0(%2)\n\tswc2 $26,4(%2)\n\tswc2 $27,8(%2)" \
    : : "r"(r0), "r"(v0), "r"(o0) : "$12", "$13", "$14", "memory")

extern short *D_8009D254;
extern int *D_800E2844;

int func_800C689C(short *a0)
{
    short *p = D_8009D254;
    int a3 = p[0x2A / 2];
    int a6 = p[0x32 / 2];
    int *s;
    int a;
    int b;

    s = (int *)0x1F800000;
    s[1] = 0;
    s[5] = 0;
    s[9] = 0;
    s[0xD] = 0;
    s[0x11] = 0;
    s[0x15] = 0;
    s[0] = a0[4] - a0[0];
    s[2] = a0[6] - a0[2];
    D_800E2844 = s;
    s[0xC] = a3 - a0[0];
    s[0xE] = a6 - a0[2];
    s = D_800E2844;
    gte_op_rowvec(s, (int *)0x1F800030, (int *)0x1F800060);
    s[4] = a0[8] - a0[4];
    s[6] = a0[0xA] - a0[6];
    s[0x10] = a3 - a0[4];
    s[0x12] = a6 - a0[6];
    s = D_800E2844;
    gte_op_rowvec(s + 4, s + 0x10, s + 0x1C);
    s[8] = a0[0] - a0[8];
    s[0xA] = a0[2] - a0[0xA];
    s[0x14] = a3 - a0[8];
    s[0x16] = a6 - a0[0xA];
    gte_op_rowvec(s + 8, s + 0x14, s + 0x20);
    s = D_800E2844;
    a = s[0x19] ^ s[0x1D];
    b = s[0x1D] ^ s[0x21];
    return (a | b) > 0;
}
