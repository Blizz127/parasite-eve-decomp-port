/*
 * func_8006D078 — CD directory-scan state machine step, driven by the state
 * byte at D_800B0CD8+0xF3 and the gp counter D_8009CDCC:
 *   0    -> reset counter, state 0x28
 *   0x28 -> read the volume descriptor (func_8006CDA4(1, 1, ...)); busy -> 1;
 *           single-session (+0x10 < 2) falls into 0x29's "state 0x2A" tail,
 *           else state 0x29 and return 1
 *   0x29 -> read session +0x10; busy -> 1; state 0x2A
 *   0x2A -> walk the 8-byte directory entries (count = word 0x24 >> 22),
 *           skipping entries without flag 0x10 or with size < 2; at the end
 *           state 0 and return 0; else state 0x2B
 *   0x2B -> read the entry's extent (func_8006CDA4(3, e->a, e->b, ...));
 *           busy -> 1; next entry, state 0x2A
 *
 * ROM: era gcc-2.7.2-psx -O2 -G8 + MASPSX_DISPATCH_FOLD=jtbl_80011458 +
 * MASPSX_THREE_WORD_SYMBOL_STORE=1 + MASPSX_FORCE_ABSOLUTE_SYMBOLS=D_800B0E64.
 * Load-bearing: a goto loop (a `for (;;)` lets loop.c hoist the 0x21 / 1
 * constants into saved registers), the shared `next:` label, and
 * `(index << 3) + (int)tab` operand order for the entry address.
 */
typedef struct {
    unsigned char flags;
    unsigned char pad1[2];
    unsigned char attr;
    unsigned short a;
    unsigned short b;
} Entry;
extern unsigned char D_800B0CD8[];
extern unsigned char *D_800B0E64;
extern int D_8009CDCC;
extern int func_8006CDA4(int, int, int, int, int, int);

int func_8006D078(void) {
    unsigned char *s = D_800B0CD8;
    unsigned char *h = D_800B0E64;
    unsigned char *dir = h + *(int *)(h + 4);
    Entry *tab = (Entry *)(h + (*(unsigned int *)(dir + 0x24) & 0x3FFFFF));
    Entry *e;

loop:
        switch (s[0xF3]) {
        case 0:
            D_8009CDCC = 0;
            s[0xF3] = 0x28;
            goto loop;
        case 0x28:
            if (func_8006CDA4(1, 1, 0, *(int *)(s + 0x194), 0x21, 0) == 1) {
                return 1;
            }
            if (s[0x10] < 2) {
                goto next;
            }
            s[0xF3] = 0x29;
            return 1;
        case 0x29:
            if (func_8006CDA4(1, s[0x10], 0, *(int *)(s + 0x194), 0x21, 0) == 1) {
                return 1;
            }
        next:
            s[0xF3] = 0x2A;
            goto loop;
        case 0x2A:
            if (D_8009CDCC < (int)(*(unsigned int *)(dir + 0x24) >> 22)) {
                Entry *p = (Entry *)((D_8009CDCC << 3) + (int)tab);
                if (!(p->attr & 0x10) || p->a < 2) {
                    D_8009CDCC++;
                } else {
                    s[0xF3] = 0x2B;
                }
                goto loop;
            }
            s[0xF3] = 0;
            return 0;
        case 0x2B:
            e = (Entry *)((D_8009CDCC << 3) + (int)tab);
            if (func_8006CDA4(3, e->a, e->b, *(int *)(s + 0x194), 0x21, 0) == 1) {
                return 1;
            }
            s[0xF3] = 0x2A;
            D_8009CDCC++;
            goto loop;
        default:
            return 0;
        }
}
