/* Phase 5DC: gp-relative leaf func_8004E970.
 * VRAM 0x8004E970 / gp+0x19C -> D_8009CF0C. gp base 0x8009CD70.
 * -G 8 emits R_MIPS_GPREL16; _gp+abs sym resolve to retail offset. */
extern int D_8009CF0C;
int func_8004E970(void){ return D_8009CF0C; }
