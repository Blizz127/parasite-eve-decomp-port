/* Phase 5DC: gp-relative leaf func_80033A20.
 * VRAM 0x80033A20 / gp+0x110 -> D_8009CE80. gp base 0x8009CD70.
 * -G 8 emits R_MIPS_GPREL16; _gp+abs sym resolve to retail offset. */
extern unsigned char D_8009CE80;
unsigned char func_80033A20(void){ return D_8009CE80; }
