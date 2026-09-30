/*
 * func_8002F970 — slot-table pointer-match search and clear (Phase 5FE,
 * table twin of 2F9CC). Scans the seven 220-byte SlotRecord entries at
 * D_800A5D58 for the record whose BODY address equals *p, clears that
 * record's in-use flag, then nulls the caller's pointer (the sw lands in
 * the jr delay slot, the 5EN store-in-jr-slot pattern).
 * SlotRecord typing inherited unchanged from func_8002F9CC.c (5FD):
 * word 0 = in-use flag, body at record+4 (labelled D_800A5D5C), extent
 * 0x604 = 7 * 220. p is `unsigned char **` because the comparison target
 * is the record body (unsigned char[216]).
 * Retail fills the back-branch delay slot with the second `andi 0xFF`
 * (inverse of 2F9CC's nop slot); the on-match inUse store is the same
 * 3-word lui $at / addu / sw %lo($at) symbolic indexed form (3W knob).
 * ROM: asm/disc1/11718.s @ file 0x20170, 23 words (0x5C bytes), no frame.
 */

typedef struct {
    /* 0x00 */ unsigned int inUse;        /* allocate sets 1; this clears */
    /* 0x04 */ unsigned char body[216];   /* payload, labelled D_800A5D5C */
} SlotRecord;                             /* 220 bytes */

extern SlotRecord D_800A5D58[];

void func_8002F970(unsigned char **p) {
    unsigned char i;

    for (i = 0; i < 7; i++) {
        if (D_800A5D58[i].body == *p) {
            D_800A5D58[i].inUse = 0;
        }
    }
    *p = 0;
}
