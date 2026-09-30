/*
 * func_80037548 — record-field lookup twin of func_800374E8 (Phase 5FH).
 * Scans the 4 x 56-byte records at D_800BCEA8 for the one whose halfword
 * offset +0x10 (signed short) equals the needle, then returns the signed
 * byte at +0x00 as a signed char; returns 0 if no record matches.
 *
 * Record type INHERITED from func_800374E8 (5FF park, D_800BCEA8 layout):
 * +0x00 unsigned char byte0, +0x0C unsigned int flags, +0x10 signed short
 * half10, 56-byte records, extent closes at +0xE0 = 4 x 56.
 *
 * THE TWIN-HYPOTHESIS TEST (5FH): 374E8 parks on register-coloring skew
 * (era colors mask->v0/chain->v1/value->v0; ROM is mask->v1/chain->v0/
 * value->v1). 37548 was predicted to hit the same skew. PROBE: REJECTED —
 * with the accumulator shape below (signed char result = 0; assign +
 * break on match), era -O2 -G0 + MASPSX_THREE_WORD_SYMBOL_STORE=1 matches
 * 27/27 words. ROM holds the return accumulator in $a2 across the scan
 * (addu $a2,$zero,$zero proxied), sign-extends the matched byte0 via
 * sll $v0,$a2,24 in the j delay slot + sra $v0,$v0,24 at the epilogue
 * merge, and NEVER re-materializes $a2 in the loop (no default-return
 * path duplication — the direct `return (signed char)` phrasing emits
 * an extra `addu $v0,$zero,$zero` before jr, 28 words). The scan counter
 * is unsigned char (two andi 0xFF + delay-slot re-mask), bound sltiu 0x4.
 * The indexed-symbolic lh/lbu pair routes through the 3-word $at gate
 * (fixed 5dac87e; bare pass-through would have used the destination.
 * lwl/lwr already expand with $at; lwc2 never routes).
 *
 * Build: era -O2 -G0 + MASPSX_THREE_WORD_SYMBOL_STORE=1.
 * ROM: asm/disc1/27D48.s @ 0x27D48 (func_80037548 runs before 374E8 in
 * the file; the 5FF carve splits 27C6C into 27C6C/374E8/27D48 where
 * 27D48.s holds 37548 + the resume), 27 words (0x6C bytes), no frame.
 */

typedef struct {
    /* 0x00 */ unsigned char byte0;
    /* 0x01 */ unsigned char pad1[0x0B];
    /* 0x0C */ unsigned int flags;
    /* 0x10 */ short half10;
    /* 0x12 */ unsigned char pad12[0x26];
} BceRecord;                              /* 0x38 = 56 bytes */

extern BceRecord D_800BCEA8[];

signed char func_80037548(short needle) {
    unsigned char i;
    signed char result = 0;

    for (i = 0; i < 4; i++) {
        if (D_800BCEA8[i].half10 == needle) {
            result = (signed char)D_800BCEA8[i].byte0;
            break;
        }
    }
    return result;
}