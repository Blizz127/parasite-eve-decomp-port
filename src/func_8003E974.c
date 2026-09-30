/* Phase 7A: func_8003E974 — VRAM 0x8003E974, size 0x154, file 0x2F174-0x2F2C8.
 * SPU/voice mask reset. Clears five gp-resident state words, zeroes the
 * 0x80-byte D_800A76F0 array, re-registers the twenty-two register-window
 * pairs with func_8003EAC8, then sets bit 0x4000 in D_8009D1A0 and returns the
 * new value. era -O2 -G8 (the five state words are gp-relative); D_8009D1A0
 * stays absolute via MASPSX_FORCE_ABSOLUTE_SYMBOLS (scalar declared extern). */
extern int D_8009D1E4;
extern int D_8009D1F4;
extern int D_8009D2D4;
extern int D_8009D238;
extern int D_8009D26C;
extern int D_800A76F0[];
extern unsigned int D_8009D1A0;
extern void func_8003EAC8(int, int);

int func_8003E974(void)
{
    int i = 0;
    unsigned int v;

    D_8009D1E4 = 0;
    D_8009D1F4 = 0;
    D_8009D2D4 = 0;
    D_8009D238 = 0;
    D_8009D26C = 0;
    do {
        D_800A76F0[i] = 0;
        i++;
    } while (i < 0x20U);
    func_8003EAC8(1, 0x4000);
    func_8003EAC8(0x80, 0x1000);
    func_8003EAC8(0x100, 0x2000);
    func_8003EAC8(8, 0x10);
    func_8003EAC8(0x20, 0x40);
    func_8003EAC8(0x40, 0x80);
    func_8003EAC8(0x10, 0x20);
    func_8003EAC8(2, 1);
    func_8003EAC8(4, 8);
    func_8003EAC8(0x200, 0x2000);
    func_8003EAC8(0x400, 0x4000);
    func_8003EAC8(0x2000, 0x8000);
    func_8003EAC8(0x01000000, 0x100);
    func_8003EAC8(0x02000000, 0x200);
    func_8003EAC8(0x04000000, 0x400);
    func_8003EAC8(0x08000000, 0x800);
    func_8003EAC8(0x10000000, 0x1000);
    func_8003EAC8(0x20000000, 0x2000);
    func_8003EAC8(0x40000000, 0x4000);
    func_8003EAC8(0x80000000, 0x8000);
    v = D_8009D1A0 | 0x4000;
    D_8009D1A0 = v;
    return v;
}
