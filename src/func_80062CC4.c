/* Phase 5DC: gp-relative leaf func_80062CC4.
 * VRAM 0x80062CC4 / gp+0x3EC -> D_8009D15C. gp base 0x8009CD70.
 * -G 8 emits R_MIPS_GPREL16; _gp+abs sym resolve to retail offset. */
extern int D_8009D15C;
int func_80062CC4(void){ return D_8009D15C; }
