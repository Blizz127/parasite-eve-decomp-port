/* ovl_0700 (PE.IMG map-exit / destination-chooser overlay, VRAM 0x8018EFF0)
 * func_80191740 — blob offset 0x2750, 0x14 bytes (5 words).
 * Evidence: docs/evidence/ovl_0700-func_80191740/REPORT.md (LINK_EXACT, era_o2_g0).
 *
 * Retail: addiu $v0,$zero,0xC8 / lui $at,%hi(D_8019CC50) /
 *         sh $v0,%lo(D_8019CC50)($at) / jr $ra / nop
 *
 * A 16-bit store of the constant 200 into the overlay's own data word at
 * 0x8019CC50 (blob offset 0xDC60, inside the CFD4.bin data tail); the symbol
 * is address-named and binds to that address at link time like the EXE lane's
 * D_ externs. Era profile: default era_o2_g0 (overlays use no $gp data). */

extern short D_8019CC50;

void func_80191740(void)
{
    D_8019CC50 = 200;
}
