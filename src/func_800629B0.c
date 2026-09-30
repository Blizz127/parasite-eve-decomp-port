/* Phase 5DC: gp-relative leaf func_800629B0.
 * VRAM 0x800629B0 / gp+0x3E4 -> D_8009D154. gp base 0x8009CD70.
 * -G 8 emits R_MIPS_GPREL16; _gp+abs sym resolve to retail offset. */
extern int D_8009D154;
int func_800629B0(void){ return D_8009D154 != 0; }
