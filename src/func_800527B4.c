/* Phase 5DC: gp-relative leaf func_800527B4.
 * VRAM 0x800527B4 / gp+0x2B0 -> D_8009D020. gp base 0x8009CD70.
 * -G 8 emits R_MIPS_GPREL16; _gp+abs sym resolve to retail offset. */
extern int D_8009D020;
int func_800527B4(void){ return D_8009D020; }
