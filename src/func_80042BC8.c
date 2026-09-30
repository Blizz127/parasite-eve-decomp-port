/* Phase 5BP: sixty-fifth matching C leaf (mid-33338 carve).
 * VRAM 0x80042BC8 / file 0x333C8 / size 0x10.
 * Original: lui $v0,%hi(D_800A1870); lw $v0,%lo(D_800A1870)($v0);
 *           jr $ra; sltu $v0,$zero,$v0
 * Boolean nonzero test of D_800A1870.
 *
 * Type corrected: D_800A1870 is a function pointer (proven by func_80042B6C
 * jalr $a0), not int. Nonzero test still matches the same words.
 */
typedef void (*func_800A1870_t)(void);

extern func_800A1870_t D_800A1870;

int func_80042BC8(void) {
    return D_800A1870 != 0;
}
