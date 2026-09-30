/* func_800DFB78 — VRAM 0x800DFB78 / size 0xCC. LINK_EXACT, era -O2 -G0.
 * Actor-flag gate. obj stays in $v1 and the +0xC sub-pointer in $a1 (pins);
 * the null record falls through a goto to the shared `return 0`. The final
 * test must be `(s & 7) > 0` on a separate int, not a masked temporary —
 * that is what keeps `andi $v0,$v0,7` / `bgtz $v0`.
 */
int func_800DFB78(unsigned char *a0) {
    register unsigned char *obj asm("$3");
    register unsigned char *sub asm("$5");
    unsigned int word;
    unsigned int *rec;
    unsigned int bits;
    int s;

    obj = *(unsigned char **)(a0 + 8);
    sub = a0 + 0xC;
    if ((a0[0x19] & 4) != 0) {
        if (obj[0xE] < 2) {
            return 1;
        }
    }
    if ((sub[0xD] & 2) != 0) {
        word = *(unsigned int *)(obj + 0x98);
        if (word & 0x80000) {
            *(unsigned int *)(obj + 0x98) = word & 0xFFF7FFFF;
            return 1;
        }
    }
    if ((sub[0xD] & 1) != 0) {
        word = *(unsigned int *)(obj + 0x98);
        if (word & 0x40000) {
            *(unsigned int *)(obj + 0x98) = word & 0xFFFBFFFF;
            return 1;
        }
    }
    rec = *(unsigned int **)obj;
    if (rec == 0) {
        goto zero;
    }
    bits = *rec;
    if (bits & 0x1800) {
        return 1;
    }
    s = (int)(bits >> 1);
    if ((s & 7) > 0) {
        return 1;
    }
zero:
    return 0;
}
