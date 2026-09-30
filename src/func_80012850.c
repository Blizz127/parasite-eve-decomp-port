/*
 * func_80012850 — script VM step (24-opcode expression interpreter).
 *
 * VRAM 0x80012850 / file 0x3050 / size 0x3D0 (244 words). Frame 0x18,
 * saves $s0/$ra, calls func_8003708C/func_800370A8 (opcodes 20/21).
 * ctx fields: +0x00 pc ptr, +0x04 dst ptr, +0x08 lhs ptr, +0x0C rhs ptr.
 * Fetches *pc, range-checks < 0x18, dispatches through jtbl_80010000
 * (ascending case order confirmed from the rodata dump), executes one
 * operation storing through *dst, returns 1.
 *
 * Values are signed int (slt comparisons, srav shift, mult/div). The
 * div/mod break 7/break 6 guard sequences are the era backend's standard
 * integer division checks — emitted automatically for / and %.
 * Opcodes: 0..4 arith/bitwise, 5..6 short-circuit logicals, 7..8 unary
 * !/~, 9..14 comparisons (< > == >= <= !=), 15 mul, 16 div, 17..18
 * shifts, 19 copy, 20..21 helper calls, 22 mod, 23 negate.
 *
 * Build: era -O2 -G8 (no knobs).
 * ROM: asm/disc1/3050.s @ file 0x3050, 244 words (0x3D0 bytes).
 */

typedef struct {
    int *pc;                     /* +0x00 */
    int *dst;                    /* +0x04 */
    int *lhs;                    /* +0x08 */
    int *rhs;                    /* +0x0C */
} VmCtx;

extern int func_8003708C(int lhs, int rhs);
extern int func_800370A8(int lhs, int rhs);

int func_80012850(VmCtx *ctx) {
    switch (*ctx->pc) {
    case 0:
        *ctx->dst = *ctx->lhs + *ctx->rhs;
        break;
    case 1:
        *ctx->dst = *ctx->lhs - *ctx->rhs;
        break;
    case 2:
        *ctx->dst = *ctx->lhs | *ctx->rhs;
        break;
    case 3:
        *ctx->dst = *ctx->lhs & *ctx->rhs;
        break;
    case 4:
        *ctx->dst = *ctx->lhs ^ *ctx->rhs;
        break;
    case 5:
        *ctx->dst = (*ctx->lhs != 0) || (*ctx->rhs != 0);
        break;
    case 6:
        *ctx->dst = (*ctx->lhs != 0) && (*ctx->rhs != 0);
        break;
    case 7:
        *ctx->dst = !*ctx->lhs;
        break;
    case 8:
        *ctx->dst = ~*ctx->lhs;
        break;
    case 9:
        *ctx->dst = *ctx->lhs > *ctx->rhs;
        break;
    case 10:
        *ctx->dst = *ctx->lhs < *ctx->rhs;
        break;
    case 11:
        *ctx->dst = *ctx->lhs == *ctx->rhs;
        break;
    case 12:
        *ctx->dst = *ctx->lhs >= *ctx->rhs;
        break;
    case 13:
        *ctx->dst = *ctx->lhs <= *ctx->rhs;
        break;
    case 14:
        *ctx->dst = *ctx->lhs != *ctx->rhs;
        break;
    case 15:
        *ctx->dst = *ctx->lhs * *ctx->rhs;
        break;
    case 16:
        *ctx->dst = *ctx->lhs / *ctx->rhs;
        break;
    case 17:
        *ctx->dst = *ctx->lhs << *ctx->rhs;
        break;
    case 18:
        *ctx->dst = *ctx->lhs >> *ctx->rhs;
        break;
    case 19:
        *ctx->dst = *ctx->lhs;
        break;
    case 20:
        *ctx->dst = func_8003708C(*ctx->lhs, *ctx->rhs);
        break;
    case 21:
        *ctx->dst = func_800370A8(*ctx->lhs, *ctx->rhs);
        break;
    case 22:
        *ctx->dst = *ctx->lhs % *ctx->rhs;
        break;
    case 23:
        *ctx->dst = -*ctx->lhs;
        break;
    }
    return 1;
}
