/* VRAM 0x8004F464 / file 0x3FC64 / size 0x2C.
 * If the gp-relative word D_8009D008 (0x298($gp)) is set, call func_8004E97C
 * and clear it. era -O2 -G8. */
extern int D_8009D008;

void func_8004E97C(void);

void func_8004F464(void) {
    if (D_8009D008 != 0) {
        func_8004E97C();
        D_8009D008 = 0;
    }
}
