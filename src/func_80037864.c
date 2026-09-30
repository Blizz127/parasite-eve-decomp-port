/* Phase 5DC: gp-relative leaf func_80037864.
 * VRAM 0x80037864 / gp+0x134 -> D_8009CEA4. gp base 0x8009CD70.
 * -G 8 emits R_MIPS_GPREL16; _gp+abs sym resolve to retail offset. */
extern signed char D_8009CEA4;
int func_80037864(void){ return D_8009CEA4; }
