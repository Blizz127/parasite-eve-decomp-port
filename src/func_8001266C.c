/*
 * func_8001266C — boot work-table link init.
 *
 * VRAM 0x8001266C / file 0x2E6C / size 0x94 (37 words). Leaf, no frame,
 * no calls. Companion to func_800124F8 (same unit, same tables): fills each
 * 0x2C-byte row's link word with the address of the NEXT row start for 0x47
 * rows, nulls terminator word D_8009DF68 (= table end), clears the 0x40-word
 * array D_800B6A80, zeroes gp state word D_8009D300, seeds row-cursor
 * D_8009CDFC with the table base.
 *
 * Second attempt (first failed the trim guard ~0x9C vs 0x94; see REPORT).
 * Levers applied, per evidence + 2F9CC precedent:
 *   - real 44-byte row aggregate on BOTH symbols so cc1 keeps the symbol in
 *     the store addressing (flat unsigned int[] let it hoist the base) and
 *     scales &row[i+1] by 44;
 *   - address-only RHS (&D_8009D310[i+1]) so the stored value is computed,
 *     not loaded; invariant part folds to addiu a1,v0,0x2C.
 *
 * Counter is unsigned short (andi 0xFFFF masks at every use; sltiu bounds
 * 0x47 / 0x40). The indexed symbolic store routes through the THREE_WORD
 * gate (5FG precedent); the D_8009DF68 clear stays absolute via unsized-
 * array typing (124F8 D_8009DF70 rule).
 *
 * Build: era -O2 -G8 + MASPSX_THREE_WORD_SYMBOL_STORE=1.
 * ROM: asm/disc1/2E6C.s @ file 0x2E6C, 37 words (0x94 bytes).
 */

typedef struct {
    unsigned int link;       /* +0x00 — the stored-to word */
    unsigned char rest[40];  /* 44-byte row total */
} Row;

extern Row D_8009D334[];
extern Row D_8009D310[];
extern unsigned int D_8009DF68[];
extern unsigned int D_800B6A80[];
extern unsigned int D_8009D300;
extern unsigned int D_8009CDFC;

void func_8001266C(void) {
    unsigned short i;

    D_8009D300 = 0;
    D_8009CDFC = (unsigned int)D_8009D310;
    for (i = 0; i < 0x47; i++) {
        D_8009D334[i].link = (unsigned int)&D_8009D310[i + 1];
    }
    D_8009DF68[0] = 0;
    for (i = 0; i < 0x40; i++) {
        D_800B6A80[i] = 0;
    }
}
