/* Phase 5DC: gp-relative leaf func_80054288.
 * VRAM 0x80054288 / gp+0x2D0 -> D_8009D040. gp base 0x8009CD70.
 * -G 8 emits R_MIPS_GPREL16; _gp+abs sym resolve to retail offset. */
extern int D_8009D040;
int func_80054288(void){ return D_8009D040; }
