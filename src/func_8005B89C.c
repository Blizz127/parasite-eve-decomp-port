/* Phase 5DC: gp-relative leaf func_8005B89C.
 * VRAM 0x8005B89C / gp+0x2B8 -> D_8009D028. gp base 0x8009CD70.
 * -G 8 emits R_MIPS_GPREL16; _gp+abs sym resolve to retail offset. */
extern int D_8009D028;
int func_8005B89C(void){ return D_8009D028; }
