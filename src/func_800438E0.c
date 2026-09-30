/* Phase 5DC: gp-relative leaf func_800438E0.
 * VRAM 0x800438E0 / gp+0x180 -> D_8009CEF0. gp base 0x8009CD70.
 * -G 8 emits R_MIPS_GPREL16; _gp+abs sym resolve to retail offset. */
extern int D_8009CEF0;
int func_800438E0(void){ return D_8009CEF0; }
