/*
 * func_80012774 — drain matching rows from work-table buckets.
 *
 * VRAM 0x80012774 / file 0x2F74 / size 0xDC (55 words). Frameless leaf,
 * no calls. Walks the head chain at D_8009D20C (absolute load -> unsized
 * array-of-pointer typing keeps it out of gp small data); per head, walks
 * its 3 bucket lists (buckets at +0xA0, u8 bucket index with 0xFF mask,
 * sltiu bound 3). A node is removed when head->f98 (&0x10, reloaded every
 * iteration — the removal stores may alias it) or node u16 +0x08 (&0x10)
 * says so: unlink via prev->next / bucket head, fix next->prev with a
 * FRESH read of cur->next after the stores (retail reloads both), push
 * the row onto the D_8009CDFC cursor stack (row->next = old cursor), and
 * advance. Statement order mirrors the ROM sequence.
 *
 * Build: era -O2 -G8 (no knobs; no compound symbolic lines).
 * ROM: asm/disc1/2F00.s @ file 0x2F74, 55 words (0xDC bytes).
 */

typedef struct Node {
    unsigned char pad00[0x08];
    unsigned short f08;            /* +0x08 — flag u16 (& 0x10) */
    unsigned char pad0A[0x1A];     /* +0x0A..0x24 */
    struct Node *next;             /* +0x24 */
    struct Node *prev;             /* +0x28 */
} Node;

typedef struct Head {
    unsigned char pad00[0x04];
    struct Head *h_next;           /* +0x04 — next head in chain */
    unsigned char pad08[0x90];     /* +0x08..0x98 */
    unsigned int f98;              /* +0x98 — mode flags (& 0x10) */
    unsigned char pad9C[0x04];
    Node *buckets[3];              /* +0xA0 — bucket heads */
} Head;

extern void *D_8009D20C[];         /* absolute: unsized array of ptr */
extern unsigned int D_8009CDFC;    /* gp row cursor */

void func_80012774(void) {
    Head *head;
    Node *cur;
    Node *nxt;
    Node *prev;
    Node *after;
    unsigned int flags;
    unsigned int old;
    unsigned char b;

    for (head = (Head *)D_8009D20C[0]; head != 0; head = head->h_next) {
        for (b = 0; b < 3; b++) {
            cur = head->buckets[b];
            if (cur == 0) {
                continue;
            }
            do {
                flags = head->f98;
                nxt = cur->next;
                if (((flags & 0x10) != 0) || ((cur->f08 & 0x10) != 0)) {
                    prev = cur->prev;
                    if (prev != 0) {
                        prev->next = nxt;
                    } else {
                        head->buckets[b] = nxt;
                    }
                    after = cur->next;
                    if (after != 0) {
                        after->prev = cur->prev;
                    }
                    old = D_8009CDFC;
                    D_8009CDFC = (unsigned int)cur;
                    cur->next = (Node *)old;
                }
                cur = nxt;
            } while (cur != 0);
        }
    }
}
