/* Phase 5DC: gp-relative leaf func_80042CB8.
 * VRAM 0x80042CB8 / gp+0x17C -> D_8009CEEC. gp base 0x8009CD70.
 * -G 8 emits R_MIPS_GPREL16; _gp+abs sym resolve to retail offset. */
extern int D_8009CEEC;
void func_80042CB8(int a0){ D_8009CEEC = a0; }
