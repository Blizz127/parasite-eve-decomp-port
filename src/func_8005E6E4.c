/* Phase 5DC: gp-relative leaf func_8005E6E4.
 * VRAM 0x8005E6E4 / gp+0x3C4 -> D_8009D134. gp base 0x8009CD70.
 * -G 8 emits R_MIPS_GPREL16; _gp+abs sym resolve to retail offset. */
extern int D_8009D134;
void func_8005E6E4(int a0){ D_8009D134 = a0; }
