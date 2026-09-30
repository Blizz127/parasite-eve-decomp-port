/* Phase 5DC: gp-relative leaf func_8005EB58.
 * VRAM 0x8005EB58 / gp+0x39C -> D_8009D10C. gp base 0x8009CD70.
 * -G 8 emits R_MIPS_GPREL16; _gp+abs sym resolve to retail offset. */
extern int D_8009D10C;
void func_8005EB58(int a0){ D_8009D10C = a0; }
