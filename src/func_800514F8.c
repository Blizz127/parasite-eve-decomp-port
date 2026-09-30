/* Phase 5DC: gp-relative leaf func_800514F8.
 * VRAM 0x800514F8 / gp+0x2A0 -> D_8009D010. gp base 0x8009CD70.
 * -G 8 emits R_MIPS_GPREL16; _gp+abs sym resolve to retail offset. */
extern int D_8009D010;
int func_800514F8(void){ return D_8009D010; }
