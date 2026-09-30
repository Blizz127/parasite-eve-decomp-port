/* Phase 5DC: gp-relative leaf func_8005E120.
 * VRAM 0x8005E120 / gp+0x384 -> D_8009D0F4. gp base 0x8009CD70.
 * -G 8 emits R_MIPS_GPREL16; _gp+abs sym resolve to retail offset. */
extern int D_8009D0F4;
int func_8005E120(void){ return D_8009D0F4; }
