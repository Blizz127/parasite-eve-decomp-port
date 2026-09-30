/* Phase 5DC: gp-relative leaf func_8005E57C.
 * VRAM 0x8005E57C / gp+0x3B0 -> D_8009D120. gp base 0x8009CD70.
 * -G 8 emits R_MIPS_GPREL16; _gp+abs sym resolve to retail offset. */
extern int D_8009D120;
void func_8005E57C(int a0){ D_8009D120 = a0; }
