/*
 * func_800363F4 — search-and-clear over the 16-entry D_800A7624 table
 * (Phase 5FG). Scans for the entry whose key word (offset +0) equals the
 * needle argument, clears that key to 0, and breaks immediately. If no
 * match, the 16 entries are scanned and nothing is cleared.
 *
 * Table shape from func_800362B8 (same unit): 8-byte strides (sll $a1,$v0,3)
 * over D_800A7624 with the key at +0 (lw/sw), a +4 partner word (the writer
 * stores a pointer-linked address there), and D_800A7620 as a companion
 * table sharing the same 8-byte stride. The 20-entry companion write site
 * (sll $a1,$v0,3 / lui $at / addu / sw %lo) confirms the 8-byte element.
 *
 * Counter is unsigned char (three andi 0xFF sites: loop-head mask, test
 * mask, and the delay-slot re-mask because the table load clobbers $v0).
 * Bound test is sltiu vs 0x10 -> unsigned compare (3E680 rule). Search #1
 * (this function) and the writer loop func_800362B8's inner search both use
 * the same 3-word indexed-symbolic lw/sw pair — retail assembled with the
 * ASPSX 2.30 form throughout, so BOTH loads and stores must route through
 * MASPSX_THREE_WORD_SYMBOL_STORE=1.
 *
 * $at-temp detail (ROM-proven at 363F4 + 36448): the indexed-symbolic LOAD
 * uses $at as the address temp (lui $at / addu $at,$at,$a1 / lw %lo($at)).
 * A bare pass-through would let GNU as pick the DESTINATION register
 * (lui $v0 / addu $v0,$v0,$a1 / lw $v0,0($v0)) — the 439c244 load-gate bug,
 * fixed at 5dac87e. Stores already use $at on both paths.
 *
 * Build: era -O2 -G0 + MASPSX_THREE_WORD_SYMBOL_STORE=1 (both the indexed
 * load and the indexed store route through the gate).
 * ROM: asm/disc1/2422C.s @ file 0x26BF4, 21 words (0x54 bytes), no frame.
 */

typedef struct {
    unsigned int key;    /* D_800A7624: lw/sw word, compared unsign-extended */
    unsigned int pad4;   /* +4 partner word; 8-byte stride from sll $a1,$v0,3 */
} SearchEntry;           /* 8 bytes */

extern SearchEntry D_800A7624[];

void func_800363F4(unsigned int needle) {
    unsigned char i;

    for (i = 0; i < 16; i++) {
        if (D_800A7624[i].key == needle) {
            D_800A7624[i].key = 0;
            break;
        }
    }
}