/* Phase 5DC: gp-relative leaf func_80064A48.
 * VRAM 0x80064A48 / gp+0x3FC -> D_8009D16C. gp base 0x8009CD70.
 * -G 8 emits R_MIPS_GPREL16; _gp+abs sym resolve to retail offset. */
extern int D_8009D16C;
int func_80064A48(void){ return D_8009D16C; }
