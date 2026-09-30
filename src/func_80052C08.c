/* Phase 5FZ: matching C leaf.
 * VRAM 0x80052C08 / file 0x43408 / size 0x64 (25 words).
 *
 * Appends a 0xFF-terminated byte string onto the end of another.  The scan to the
 * destination's terminator is written as a post-increment test plus an explicit
 * step back — `while (*dst++ != 0xFF) { } dst--;` — which is what retail emits:
 * the frame has the `addiu dst,dst,1` in the branch delay slot of the *exit*
 * comparison, leaving the pointer one past the terminator before the step back.
 * The append then copies bytes (terminator included) the same way.
 *
 * A pre-increment spelling (`while (*dst != 0xFF) dst++;` plus
 * `while ((*dst++ = *src++) != 0xFF) { }`) reads identically but rotates both
 * loops differently: 13 word mismatches at the same size class.
 */
void func_80052C08(unsigned char *dst, unsigned char *src) {
    while (*dst++ != 0xFF) { }
    dst--;

    while ((*dst++ = *src++) != 0xFF) { }
}
