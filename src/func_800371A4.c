/* Phase 5DC: gp-relative leaf func_800371A4.
 * VRAM 0x800371A4 / gp+0x124 -> D_8009CE94. gp base 0x8009CD70.
 * -G 8 emits R_MIPS_GPREL16; _gp+abs sym resolve to retail offset. */
extern unsigned char D_8009CE94;
void func_800371A4(int a0){ D_8009CE94 = (unsigned char)a0; }
