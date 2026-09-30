/* READY-FROM-READER dual clear: D_800A1870 = NULL, D_800A1874 = 0.
 * VRAM 0x80042B38 / file 0x33338 / size 0x18.
 * Era path: gcc-2.7.2-psx -O2 -G0 + maspsx 2.21 --dont-expand-li.
 *
 * Types from func_80042B6C: D_800A1870 = void (*)(void) (jalr proof);
 * D_800A1874 = int (+1 counter). Store width sw matches both.
 */
typedef void (*func_800A1870_t)(void);

extern func_800A1870_t D_800A1870;
extern int D_800A1874;

void func_80042B38(void) {
    D_800A1870 = 0;
    D_800A1874 = 0;
}
