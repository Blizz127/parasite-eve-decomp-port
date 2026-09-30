/* Phase 5DC: gp-relative leaf func_800614A0.
 * VRAM 0x800614A0 / gp+0x3DC -> D_8009D14C. gp base 0x8009CD70.
 * -G 8 emits R_MIPS_GPREL16; _gp+abs sym resolve to retail offset. */
extern int D_8009D14C;
int func_800614A0(void){ return D_8009D14C; }
