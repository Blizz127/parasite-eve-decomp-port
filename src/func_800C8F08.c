/* Phase 5EC: era sb+ret0 twin of func_800C7DC4.
 * VRAM 0x800C8F08 / file 0xB9708 / size 0x10.
 * Original: addiu $v0,4; sb $v0,0($a0); jr; addu $v0,$zero,$zero
 * Compiled via era_compile -O2 -G0. */
int func_800C8F08(unsigned char *arg0) {
    *arg0 = 4;
    return 0;
}
