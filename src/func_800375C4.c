/* Phase 5DC: gp-relative leaf func_800375C4.
 * VRAM 0x800375C4 / gp+0x160 -> D_8009CED0. gp base 0x8009CD70.
 * -G 8 emits R_MIPS_GPREL16; _gp+abs sym resolve to retail offset. */
extern unsigned char D_8009CED0;
void func_800375C4(void){ D_8009CED0 = 0; }
