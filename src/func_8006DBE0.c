/*
 * func_8006DBE0 — search two tagged byte pairs; return the pair index, or -1.
 *
 * VRAM 0x8006DBE0 / file 0x5E3E0 / size 0x38 (14 words).
 *
 * Retail (asm/disc1/5E39C.s): signed-byte compare of D_800B0CD8+0xDC/0xDE
 * against a0; on a match returns the loop index (0 or 1), else -1.
 *
 * Build: era -O2 -G0. Indexing the aggregate member (`t->tag[i][0]`) keeps the
 * 0xDC displacement on the load and allocates `$a1` base / `$v1` counter as
 * retail (see func_8006DB9C).
 * ROM: asm/disc1/5E39C.s @ file 0x5E3E0, 14 words (0x38 bytes).
 */

typedef struct {
    unsigned char pad[0xDC];
    signed char tag[2][2];
} Tag;

extern Tag D_800B0CD8;

int func_8006DBE0(int a0) {
    Tag *t = &D_800B0CD8;
    int i;

    for (i = 0; i < 2; i++) {
        if (t->tag[i][0] == a0)
            return i;
    }
    return -1;
}
