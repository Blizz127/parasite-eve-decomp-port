/* Phase 5DC: gp-relative leaf func_80042F38.
 * VRAM 0x80042F38 / gp+0x168 -> D_8009CED8. gp base 0x8009CD70.
 * -G 8 emits R_MIPS_GPREL16; _gp+abs sym resolve to retail offset. */
extern int D_8009CED8;
void func_80042F38(void){ D_8009CED8 = 0; }
