/* Phase 5DC: gp-relative leaf func_80043240.
 * VRAM 0x80043240 / gp+0x1CC -> D_8009CF3C. gp base 0x8009CD70.
 * -G 8 emits R_MIPS_GPREL16; _gp+abs sym resolve to retail offset. */
extern int D_8009CF3C;
void func_80043240(int a0){ D_8009CF3C = a0; }
