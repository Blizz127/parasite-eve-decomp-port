/* Phase 5DC: gp-relative leaf func_8005BCB0.
 * VRAM 0x8005BCB0 / gp+0x4A8 -> D_8009D218. gp base 0x8009CD70.
 * -G 8 emits R_MIPS_GPREL16; _gp+abs sym resolve to retail offset. */
extern int D_8009D218;
int func_8005BCB0(void){ return D_8009D218; }
