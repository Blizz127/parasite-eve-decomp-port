/*
 * func_8006BE4C — mount "state changed" flag setter (retail 0x8006BE4C).
 *
 * VRAM 0x8006BE4C / file 0x5C64C / size 0x80 (32 words), inside 5B1E4.s.
 *
 * Reads the current mount state byte D_800B0CE2 and the previous one at
 * D_800B0CE3. Only when the live state is one of 0xA..0xE (unsigned
 * `t - 0xA < 5`) does it touch D_800B0CD8: if the two state bytes differ
 * it sets bit 0x200000 in the flag word. Independently it compares the
 * halved state against the halved previous state and, on mismatch, sets
 * bit 4 in the byte flag D_800B0CE6. Always returns 0.
 *
 * era -O2 -G0. The `unsigned int *p = &D_800B0CD8` local is required:
 * with a plain global read-modify-write cc1 emits `lui $v0,%hi` /
 * `lui $at,%hi` for the load and store (2 extra words, branch offsets
 * shift); the pointer makes cc1 materialize the base once into $a2 with
 * `lui`/`addiu` and use `lw/sw 0($a2)`, exactly like retail.
 */
extern unsigned char D_800B0CE2;
extern unsigned char D_800B0CE3;
extern unsigned int D_800B0CD8;
extern unsigned char D_800B0CE6;

int func_8006BE4C(void) {
    unsigned int *p = &D_800B0CD8;
    unsigned int t = D_800B0CE2 - 0xA;
    if (t < 5) {
        if (D_800B0CE2 != D_800B0CE3) {
            *p |= 0x200000;
        }
        if ((t >> 1) != (int)(D_800B0CE3 - 0xA) / 2) {
            D_800B0CE6 |= 4;
        }
    }
    return 0;
}
