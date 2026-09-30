/* Phase 5DC: gp-relative leaf func_8005BEDC.
 * VRAM 0x8005BEDC / gp+0x350 -> D_8009D0C0. gp base 0x8009CD70.
 * -G 8 emits R_MIPS_GPREL16; _gp+abs sym resolve to retail offset. */
extern int D_8009D0C0;
int func_8005BEDC(void){ return D_8009D0C0; }
