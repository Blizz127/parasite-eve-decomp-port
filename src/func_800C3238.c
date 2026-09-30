/* VRAM 0x800C3238 / file 0xB3A38 / size 0xEC.
 * Writes the selector byte D_800F33B8, maps selector 0..4 to the
 * D_800F337A / D_800E224C pair, then calls
 * func_80077A64(D_800F33AC, D_800E224C, D_800F3424, D_800F3426) and stores
 * the halfword result in D_800E27AC. era_o2_g0. */
extern unsigned char D_800F33B8;
extern unsigned char D_800F337A;
extern unsigned char D_800E224C;
extern unsigned char D_800F33AC;
extern unsigned short D_800F3424;
extern unsigned short D_800F3426;
extern unsigned short D_800E27AC;

int func_80077A64();

void func_800C3238(int arg0) {
    D_800F33B8 = arg0;
    switch (arg0 & 0xFF) {
    case 0:
        D_800F337A = 0;
        D_800E224C = 0;
        break;
    case 1:
        D_800F337A = 1;
        D_800E224C = 0;
        break;
    case 2:
        D_800F337A = 1;
        D_800E224C = 1;
        break;
    case 3:
        D_800F337A = 1;
        D_800E224C = 2;
        break;
    case 4:
        D_800F337A = 1;
        D_800E224C = 3;
        break;
    }
    D_800E27AC = func_80077A64(D_800F33AC, D_800E224C, D_800F3424,
                               D_800F3426);
}
