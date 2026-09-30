/* Phase 5ED: era sb+ret0 batch (shape proven 5EC).
 * VRAM 0x800CA798 / file 0xBAF98 / size 0x10.
 * Original: addiu $v0,4; sb $v0,0($a0); jr; addu $v0,$zero,$zero
 * Compiled via era_compile -O2 -G0. */
int func_800CA798(unsigned char *arg0) {
    *arg0 = 4;
    return 0;
}
