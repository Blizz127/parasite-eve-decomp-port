/* Pre-jr $at dual-store setter (5EG harvest payoff for reader func_8008AB1C).
 * VRAM 0x80085728 / file 0x75F28 / size 0x1C.
 * Era path: gcc-2.7.2-psx -O2 -G0 + maspsx 2.21 --dont-expand-li.
 *
 * Proven types (func_8008AB1C):
 *   D_8009D240 : unsigned short *
 *   D_8009D260 : unsigned char *
 *
 * Body: store base as the ushort table pointer, then store base+0x800 (byte
 * addiu) as the uchar buffer base; jr/nop. Same dual-buffer layout the reader
 * consumes (table at D_8009D240, data at D_8009D260).
 */
extern unsigned short *D_8009D240;
extern unsigned char *D_8009D260;

void func_80085728(unsigned short *base) {
    D_8009D240 = base;
    D_8009D260 = (unsigned char *)base + 0x800;
}
