/* Phase 5DC: gp-relative leaf func_80051504.
 * VRAM 0x80051504 / gp+0x2A0 -> D_8009D010. gp base 0x8009CD70.
 * -G 8 emits R_MIPS_GPREL16; _gp+abs sym resolve to retail offset. */
extern int D_8009D010;
void func_80051504(void){ D_8009D010 = 0; }
