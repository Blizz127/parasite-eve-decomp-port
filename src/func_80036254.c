/*
 * func_80036254 — VRAM 0x80036254 / file 0x26A54 / size 0x64 (25 words).
 * Walks the three task lists anchored at actor+0xA0; for every node with a
 * non-null +0x04 link it re-points +0x00 at it, sets +0x10 to 1 and clears
 * bits 5-6 of the +0x08 halfword flags, following the +0x24 next links.
 * Build: era -O2 -G8.
 */

typedef struct Node Node;
struct Node {
    int f0;                   /* +0x00 */
    Node *f4;                 /* +0x04 */
    unsigned short f8;        /* +0x08 */
    char pad0A[6];            /* 0x0A..0x0F */
    int f10;                  /* +0x10 */
    char pad14[0x10];         /* 0x14..0x23 */
    Node *f24;                /* +0x24 */
};

void func_80036254(char *a0) {
    unsigned int i;
    Node *p;

    for (i = 0; i < 3; i++) {
        p = *(Node **)(a0 + i * 4 + 0xA0);
        while (p != 0) {
            if (p->f4 != 0) {
                p->f0 = (int)p->f4;
                p->f10 = 1;
                p->f8 &= 0xFF9F;
            }
            p = p->f24;
        }
    }
}
