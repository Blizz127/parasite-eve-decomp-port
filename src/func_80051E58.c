/* Phase 5DC: gp-relative leaf func_80051E58.
 * VRAM 0x80051E58 / gp+0x2A8 -> D_8009D018. gp base 0x8009CD70.
 * -G 8 emits R_MIPS_GPREL16; _gp+abs sym resolve to retail offset. */
extern int D_8009D018;
int func_80051E58(void){ return D_8009D018; }
