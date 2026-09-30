/* Phase 5DC: gp-relative leaf func_80062CB8.
 * VRAM 0x80062CB8 / gp+0x3EC -> D_8009D15C. gp base 0x8009CD70.
 * -G 8 emits R_MIPS_GPREL16; _gp+abs sym resolve to retail offset. */
extern int D_8009D15C;
void func_80062CB8(int a0){ D_8009D15C = a0; }
