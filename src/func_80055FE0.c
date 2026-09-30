/* VRAM 0x80055FE0 / file 0x467E0 / size 0x2C.
 * gp-relative D_8009D058 bit test: (D_8009D058[index >> 5] &
 * (1 << (index & 0x1F))) != 0.  era -O2 -G8 keeps the table pointer load
 * gp-relative (0x2E8($gp)). */
extern unsigned int *D_8009D058;

int func_80055FE0(int index) {
    return (D_8009D058[index >> 5] & (1 << (index & 0x1F))) != 0;
}
