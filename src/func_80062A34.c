/*
 * func_80062A34 — 2-key node-list search (Phase 5FI candidate, 21 words).
 * Walks a singly-linked list whose head is the gp-relative global
 * D_8009D154 (gp+0x3E4, _gp=0x8009CD70) and returns the first node whose
 * +0x20 word equals arg0 AND +0x24 word equals arg1; NULL if the list is
 * empty, exhausted, or no node matches.
 *
 * Node struct Stage-0 (this unit + sibling cluster 52ABC/531BC/534D0):
 *   +0x00 next  — next-pointer (lw $v1,0x0($v1); bnez in the walk)
 *   +0x04..+0x1F pad (unused by this leaf)
 *   +0x20 int   — compared lw $v0,0x20($v1) vs $a0 (full-width bne)
 *   +0x24 int   — compared lw $v0,0x24($v1) vs $a1 (full-width beq)
 * Both keys compare raw full-width words (no mask/sign-extend at the use),
 * so `int` args are codegen-identical to unsigned. The head global is
 * already declared `extern int` in func_800629B0.c; this TU's `Node *`
 * extern is an explicit pointer-type reference (dereferenced here) — the
 * two TUs disagree on the C type but share the same address at link, and
 * 629B0 only does `!= 0` so its codegen is unaffected.
 *
 * Return shape (37548 accumulator lesson): BOTH exits — the null-head
 * short-circuit and the match — merge at a single `jr $ra / addu
 * $v0,$v1,$zero` (return node). One `return node;` at the end; a direct
 * return-in-loop would duplicate the merge.
 *
 * Live-value count: 2 in the loop ($v1 node, $v0 scratch; $a0/$a1 are
 * args) — well under the 3+ coloring-skew threshold. No calls. No
 * constant materialization (no -O1 signal). gp head access needs -G8.
 *
 * Build: era -O2 -G8 (small-data threshold for the gp-relative head).
 * ROM: asm/disc1/531BC.s @ file 0x53234, 21 words (0x54 bytes), no frame.
 */

typedef struct Node {
    struct Node *next;        /* +0x00 */
    unsigned char pad4[0x1C]; /* +0x04..+0x1F */
    int key20;                /* +0x20 */
    int key24;                /* +0x24 */
} Node;

extern Node *D_8009D154;

Node *func_80062A34(int key0, int key1) {
    Node *node = D_8009D154;

    while (node) {
        if (node->key20 == key0 && node->key24 == key1) {
            break;
        }
        node = node->next;
    }
    return node;
}