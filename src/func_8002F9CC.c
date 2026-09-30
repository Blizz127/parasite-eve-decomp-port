/*
 * func_8002F9CC — slot-table clear (Phase 5FD volume leaf). Frees all seven
 * entries of the 220-byte record table based at D_800A5D58 by zeroing each
 * record's in-use flag.
 *
 * Stage-0 typing from READERS, not from this writer. func_8002F7xx
 * (asm/disc1/11718.s @ 0x2002C-0x2007C) walks the SAME 220-byte stride chain
 * and shows the record shape:
 *   - `lw` D_800A5D58 + 220*i then `bnez` -> word 0 is an in-use flag,
 *     and `sw $t4`(=1) into it on claim: allocate sets, this function clears.
 *   - `addu $a3,$v1,$t2` with $t2 = &D_800A5D5C forms the record BODY pointer
 *     at record+4, which is then written through (`sw $v1,0x4($a3)` etc.).
 *   - D_800A5D5C's data extent runs to the next symbol D_800A6360:
 *         0x800A6360 - 0x800A5D5C = 0x604 = 1540 = 7 * 220
 *     confirming 7 records of 220 bytes.
 * Hence `struct { unsigned int inUse; unsigned char body[216]; }` x7.
 *
 * ROM stride chain (asm/disc1/11718.s, file 0x201CC):
 *     andi $v0,$a0,0xFF        v0 = i & 0xFF
 *     sll  $v1,$v0,3           8*v0
 *     subu $v1,$v1,$v0         7*v0
 *     sll  $v1,$v1,3          56*v0
 *     subu $v1,$v1,$v0        55*v0
 *     sll  $v1,$v1,2         220*v0
 * i.e. the canonical gcc expansion of *220 == 4 * (8*(8-1) - 1). The stride is
 * 220 bytes / 55 words -- NOT 196/49. The C writes the natural subscript and
 * lets strength reduction produce the chain.
 *
 * Address shape is why the element must be the real 220-byte aggregate rather
 * than a flat word array with a computed index. Retail keeps symbol+register
 * at the store site (`sw $zero, D_800A5D58($v1)`, which the assembler expands
 * to lui $at / addu $at,$at,$v1 / sw $zero,%lo($at)). Writing this as
 * `D_800A5D58[i*55] = 0` over an `unsigned int[]` makes cc1 hoist the symbol
 * address out of the loop (`la $5,D_800A5D58`) and store through the register
 * -- 2 setup words plus an `addu`, and the store slides into the branch delay
 * slot. That residual is invariant under every ladder rung (-O2/-O1/
 * -fschedule-insns2) because it is an addressing-mode choice, not a scheduling
 * one. With the aggregate element type cc1 emits the symbol store directly.
 *
 * Counter is `unsigned char`: ROM masks with `andi 0xFF` at two sites -- once
 * feeding the index chain, once feeding the bound test -- while the raw
 * (unmasked) counter lives in $a0 across the increment. A plain `int` emits
 * neither mask. Bound test is `sltiu ... 0x7` -> unsigned compare (3E680 rule).
 * NOTE: retail's back-branch delay slot is a `nop`; the `addiu $a0,$a0,1`
 * advance sits mid-body before the bound test, not in the slot.
 *
 * Leaf, no calls, no frame (no $sp adjust), no $gp use -> -G0.
 * ROM: asm/disc1/11718.s @ file 0x201CC, 17 words (0x44 bytes).
 */

typedef struct {
    /* 0x00 */ unsigned int inUse;        /* allocate sets 1; this clears */
    /* 0x04 */ unsigned char body[216];   /* payload, labelled D_800A5D5C */
} SlotRecord;                             /* 220 bytes */

extern SlotRecord D_800A5D58[];

void func_8002F9CC(void) {
    unsigned char i;

    for (i = 0; i < 7; i++) {
        D_800A5D58[i].inUse = 0;
    }
}
