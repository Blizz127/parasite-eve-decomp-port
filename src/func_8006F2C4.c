/*
 * func_8006F2C4 — record reset by id (VRAM 0x8006F2C4, file 0x5FAC4,
 * 54 words / 0xD8).
 *
 * Resolves the record for `idx` in the two id-indexed arenas and clears it:
 *   - ids 0x00..0x0A live in D_800942E4 with stride 0xA0C;
 *   - ids 0x0B..0x15 live in D_800942E8 with stride 0x10C.
 * If the record's language/handler byte (offset 1) is 0x72, it first zeroes
 * seven words of D_800E10A0 (the voice table) and clears bit 16 of the
 * D_800B0CD8 volume flags. Then bytes 0..3 become 00 FF FF FF and the two
 * words at +4/+8 are zeroed. Returns 0, or -1 for an out-of-range id.
 *
 * Both arenas are POINTER globals (retail loads the base with lui+lw), and
 * the stride multiply is emitted from the byte-offset expression, so the
 * arenas are declared `unsigned char *` and addressed with `base + i*stride`.
 *
 * ROM: era gcc-2.7.2-psx -O2 -G0.
 */
extern unsigned char *D_800942E4;
extern unsigned char *D_800942E8;
extern int D_800E10A0[];
extern unsigned int D_800B0CD8;

int func_8006F2C4(int idx)
{
    unsigned char *p;

    if ((unsigned int)idx >= 0x16)
        return -1;

    if ((unsigned int)idx >= 0xB)
        p = D_800942E8 + (idx - 0xB) * 0x10C;
    else
        p = D_800942E4 + idx * 0xA0C;

    if (p[1] == 0x72) {
        unsigned int i;
        unsigned int *flagsPtr;

        for (i = 0x6C; i < 0x73; i++)
            D_800E10A0[i - 0x6C] = 0;
        flagsPtr = &D_800B0CD8;
        *flagsPtr &= 0xFFFEFFFF;
    }

    p[0] = 0;
    p[1] = 0xFF;
    p[2] = 0xFF;
    p[3] = 0xFF;
    *(int *)(p + 4) = 0;
    *(int *)(p + 8) = 0;
    return 0;
}
