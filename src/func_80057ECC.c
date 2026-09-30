/* Phase 5DC: gp-relative leaf func_80057ECC.
 * VRAM 0x80057ECC / gp+0x308 -> D_8009D078. gp base 0x8009CD70.
 * -G 8 emits R_MIPS_GPREL16; _gp+abs sym resolve to retail offset. */
extern int D_8009D078;
int func_80057ECC(void){ return D_8009D078; }
