/*
 * func_800631DC — null-safe free-list search (Phase 5FI candidate, 20 words).
 *
 * Walks the gp-relative head D_8009D154 (gp+0x3E4, _gp=0x8009CD70) and returns
 * the last node whose +0x20 word is 1 AND whose +0x44 word is nonzero; 0 when
 * the list is empty or nothing matches. The `type == 1` compare is hoisted
 * (`li $a1,1`) and the result accumulator (`move $a0,$zero`) is seeded before
 * the loop, so both live across the walk.
 *
 * ROM: asm/disc1/539DC.s @ file 0x539DC, 20 words (0x50 bytes), no frame.
 */

typedef struct Node {
    struct Node *next;          /* +0x00 */
    unsigned char pad4[0x1C];   /* +0x04..+0x1F */
    int type;                   /* +0x20 */
    unsigned char pad24[0x20];  /* +0x24..+0x43 */
    unsigned int flag;          /* +0x44 */
} Node;

extern Node *D_8009D154;

Node *func_800631DC(void) {
    Node *result = 0;
    Node *node = D_8009D154;

    while (node) {
        if (node->type == 1 && node->flag != 0) {
            result = node;
        }
        node = node->next;
    }
    return result;
}
