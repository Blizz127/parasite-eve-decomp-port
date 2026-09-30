/* Phase 5DC: gp-relative leaf func_800622B0.
 * VRAM 0x800622B0 / gp+0x3C0 -> D_8009D130. gp base 0x8009CD70.
 * -G 8 emits R_MIPS_GPREL16; _gp+abs sym resolve to retail offset. */
extern int D_8009D130;
void func_800622B0(int a0){ D_8009D130 = a0; }
