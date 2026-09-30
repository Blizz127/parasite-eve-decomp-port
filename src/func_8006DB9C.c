/*
 * func_8006DB9C — search two tagged byte pairs; return the second byte of the
 * matching pair, or -1.
 *
 * VRAM 0x8006DB9C / file 0x5E39C / size 0x44 (17 words).
 *
 * Retail (asm/disc1/5E39C.s): signed-byte compare of D_800B0CD8+0xDC/0xDE
 * against a0; on a match returns the byte at the pair's +1 (signed), else -1.
 *
 * Build: era -O2 -G0. Indexing the aggregate member (`t->tag[i][0]`) keeps the
 * 0xDC displacement on the load and allocates `$v1` base / `$a1` counter as
 * retail; a `signed char *` walk folds the displacement into the base.
 * ROM: asm/disc1/5E39C.s @ file 0x5E39C, 17 words (0x44 bytes).
 */

typedef struct {
    unsigned char pad[0xDC];
    signed char tag[2][2];
} Tag;

extern Tag D_800B0CD8;

int func_8006DB9C(int a0) {
    Tag *t = &D_800B0CD8;
    int i;

    for (i = 0; i < 2; i++) {
        if (t->tag[i][0] == a0)
            return t->tag[i][1];
    }
    return -1;
}
