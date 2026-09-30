/* Phase 5DC: gp-relative leaf func_80054294.
 * VRAM 0x80054294 / gp+0x2F8 -> D_8009D068. gp base 0x8009CD70.
 * -G 8 emits R_MIPS_GPREL16; _gp+abs sym resolve to retail offset. */
extern int D_8009D068;
int func_80054294(void){ return D_8009D068; }
