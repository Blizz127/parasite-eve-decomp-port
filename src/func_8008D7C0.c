/* Phase 5BV: seventy-first matching C leaf (mid-7D284 carve).
 * VRAM 0x8008D7C0 / file 0x7DFC0 / size 0x10.
 * Original: lui/lw/jr/sw $v0,0($a0) on D_8009B3A0.
 * Copies a 32-bit global into *a0 (lw result reused in delay-slot sw).
 */
extern int D_8009B3A0;

void func_8008D7C0(int *a0) {
    *a0 = D_8009B3A0;
}
