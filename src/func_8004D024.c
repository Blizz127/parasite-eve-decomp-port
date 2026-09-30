/* Phase 5DC: gp-relative leaf func_8004D024.
 * VRAM 0x8004D024 / gp+0x28C -> D_8009CFFC. gp base 0x8009CD70.
 * -G 8 emits R_MIPS_GPREL16; _gp+abs sym resolve to retail offset. */
extern int D_8009CFFC;
void func_8004D024(int a0){ D_8009CFFC = a0; }
