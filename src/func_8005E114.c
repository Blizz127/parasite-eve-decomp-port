/* Phase 5DC: gp-relative leaf func_8005E114.
 * VRAM 0x8005E114 / gp+0x37C -> D_8009D0EC. gp base 0x8009CD70.
 * -G 8 emits R_MIPS_GPREL16; _gp+abs sym resolve to retail offset. */
extern int D_8009D0EC;
void func_8005E114(int a0){ D_8009D0EC = a0; }
