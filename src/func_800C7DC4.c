/* Phase 5EC: era sb+ret0 rematch of Phase 5I blocker.
 * VRAM 0x800C7DC4 / file 0xB85C4 / size 0x10.
 * Original: addiu $v0,4; sb $v0,0($a0); jr; addu $v0,$zero,$zero
 * Compiled via era_compile -O2 -G0. */
int func_800C7DC4(unsigned char *arg0) {
    *arg0 = 4;
    return 0;
}
