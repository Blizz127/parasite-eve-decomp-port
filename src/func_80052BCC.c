/* Phase 5EW: string copy-until-0xFF with rotated (head-peeled) loop.
 * VRAM 0x80052BCC / file 0x433CC / size 0x3C (15 words).
 * Era path: gcc-2.7.2-psx -O1 -G0 -fschedule-insns2 + maspsx 2.21.
 *
 * Phrasing constraints (each is load-bearing, verified 15/15):
 *  - Two explicit const vars: u8 `t` for the head compare dies at the guard
 *    (its $v0 is reused by the loop byte), forcing the loop's const to
 *    re-materialize into $v1 (retail's reload word). An int head const lives
 *    through CSE into the loop (no reload); comparing the loop byte against a
 *    variable instead of the 0xFF literal cross-jumps head and loop bodies.
 *  - `int b` in the loop (from unsigned char src): zero-extended load, so the
 *    compare needs no mask — retail's raw bne. A char/u8 loop var emits an
 *    extra andi at -O1.
 *  - `char c` head byte + explicit `c &= 0xFF`: in-place andi on $v1.
 *  - -fschedule-insns2 (post-alloc scheduler) hoists the head's li above the
 *    sb, filling the lbu delay exactly as retail's ccpsx did; -O1 alone
 *    emits it after the andi (14/15). This is the first leaf using it.
 */
void func_80052BCC(char *dst, unsigned char *src) {
    char c = *src++;
    unsigned char t = 0xFF;
    int b;
    *dst++ = c;
    c &= 0xFF;
    if (c == t) return;
    do {
        b = *src++;
        *dst++ = b;
    } while (b != 0xFF);
}
