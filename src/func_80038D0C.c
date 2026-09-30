/* Phase 5BQ: sixty-sixth matching C leaf (mid-98BC carve).
 * VRAM 0x80038D0C / file 0x2950C / size 0x10.
 * Original: lui $v0,%hi(D_80091A1C); lbu $v0,%lo(D_80091A1C)($v0);
 *           jr $ra; sltu $v0,$zero,$v0
 */
extern unsigned char D_80091A1C;

int func_80038D0C(void) {
    return D_80091A1C != 0;
}
