/* ovl_0700 (PE.IMG map-exit overlay, VRAM 0x8018EFF0)
 * func_80191834 — blob offset 0x2844, 0x20 bytes. Era default profile era_o2_g0;
 * LINK_EXACT at the overlay VMA (docs/evidence/ovl_0700-func_80191834/REPORT.md).
 * Unlinks a node from a doubly linked list (next at +0x64, prev at +0x68). */

typedef struct Node {
    unsigned char pad[0x64];
    struct Node *next;
    struct Node *prev;
} Node;

void func_80191834(Node *n)
{
    n->next->prev = n->prev;
    n->prev->next = n->next;
}
