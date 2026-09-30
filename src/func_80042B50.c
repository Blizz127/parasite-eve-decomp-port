/* READY-FROM-READER dual store: D_800A1870 = cb, D_800A1874 = 1.
 * VRAM 0x80042B50 / file 0x33350 / size 0x1C.
 * Era path: gcc-2.7.2-psx -O2 -G0 + maspsx 2.21 --dont-expand-li.
 *
 * Types from func_80042B6C: D_800A1870 = void (*)(void); D_800A1874 = int.
 * ROM: addiu $v0,1; sw $a0,%lo(D_800A1870); sw $v0,%lo(D_800A1874); jr; nop
 */
typedef void (*func_800A1870_t)(void);

extern func_800A1870_t D_800A1870;
extern int D_800A1874;

void func_80042B50(func_800A1870_t cb) {
    D_800A1870 = cb;
    D_800A1874 = 1;
}
