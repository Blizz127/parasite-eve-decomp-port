/* Phase 5DC: gp-relative leaf func_8005EEC8.
 * VRAM 0x8005EEC8 / gp+0x40 -> D_8009CDB0. gp base 0x8009CD70.
 * -G 8 emits R_MIPS_GPREL16; _gp+abs sym resolve to retail offset. */
extern int D_8009CDB0;
void func_8005EEC8(int a0){ D_8009CDB0 = a0; }
