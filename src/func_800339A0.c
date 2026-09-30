/*
 * func_800339A0 — battle "style" row selector (retail 0x800339A0).
 *
 * VRAM 0x800339A0 / file 0x241A0 / size 0x80 (32 words). Tail of the
 * 20EE0.s unit, immediately before already-matched func_80033A20.
 *
 * Copies the 4 halfword pairs at D_80010E38 (16 bytes) onto the stack
 * with the unaligned lwl/lwr + swl/swr block shape (__builtin_memcpy on
 * a `char *` destination reproduces it exactly), indexes the local copy
 * at (style & 0xFF) * 4 and publishes the pair through the gp-relative
 * battle state: row[0] -> D_8009CE84, style -> D_8009CE80 (byte),
 * row[1] -> D_8009CE86. The row[1] value is left in $v0 on return.
 *
 * gp base 0x8009CD70, so all three are small-data (R_MIPS_GPREL16):
 * era -O2 -G8 (NOT the -G0 default) makes cc1 emit the direct
 * `sh/sb %lo($gp)` form retail uses.
 */
extern unsigned char D_80010E38[];
extern unsigned char D_8009CE80;
extern unsigned short D_8009CE84;
extern unsigned short D_8009CE86;

void func_800339A0(unsigned char style) {
    unsigned char buf[16];
    unsigned char *row;
    __builtin_memcpy(buf, D_80010E38, 16);
    row = buf + (style & 0xFF) * 4;
    D_8009CE84 = *(unsigned short *)(row + 0);
    D_8009CE86 = *(unsigned short *)(row + 2);
    D_8009CE80 = style;
}
