/* Phase 5DC: gp-relative leaf func_8004D27C.
 * VRAM 0x8004D27C / gp+0x1E0 -> D_8009CF50. gp base 0x8009CD70.
 * -G 8 emits R_MIPS_GPREL16; _gp+abs sym resolve to retail offset. */
extern int D_8009CF50;
int func_8004D27C(void){ return D_8009CF50; }
