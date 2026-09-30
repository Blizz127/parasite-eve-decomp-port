/* VRAM 0x80051CC4 / file 0x424C4 / size 0x134.
 *
 * Resource command-state initializer (decompA draft). -O2 -G8 +
 * MASPSX_THREE_WORD_SYMBOL_STORE + MASPSX_DISPATCH_FOLD=jtbl_800111F8
 * (profile era_o2_g8_dispatch_800111f8). cc1 hoists the switch table's
 * address out of the loop as `la $14,$L20`; the DISPATCH_FOLD `la` form
 * (tooling2, 2026-09-24) retargets it to the retail pool table
 * (0x80051D2C/30 `lui $t6,0x8001` / `addiu $t6,0x11F8`). Without the `la`
 * form: 2 words.
 */
extern int D_8009D018;
extern int D_800A1B30[];
extern signed char D_800C0E22[];
extern int func_80052F0C(void);
extern void func_80052E30(int);
extern unsigned char *func_8005332C(int);
extern void func_8005218C(void);

void func_80051CC4(void)
{
    int s;
    int i;
    unsigned char *q;
    int v;

    s = func_80052F0C();
    func_80052E30(0);
    D_8009D018 = 0;
    for (i = 6; i >= 0; i--) {
        D_800A1B30[i] = 0;
    }
    q = func_8005332C(D_800C0E22[0]);
    if (q != 0) {
        for (i = 0; i < q[0x14]; i++) {
            v = (q + i)[0x15] & 0x1F;
            switch (v) {
            case 8:
            case 9:
            case 10:
                D_8009D018 = 1 << (v - 8);
                break;
            case 11:
                D_800A1B30[0] = 3;
                break;
            case 12:
                D_800A1B30[1] = 2;
                break;
            case 13:
                D_800A1B30[5] = -2;
                break;
            case 15:
                D_800A1B30[1] = -2;
                break;
            }
        }
    }
    func_8005218C();
    func_80052E30(0);
    func_80052E30(s);
}
