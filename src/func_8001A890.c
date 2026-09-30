/* VRAM 0x8001A890 / file 0xB090 / size 0x88.
 * Bootstrap state clear: one word at D_8009CE08, two interleaved halfword
 * arrays D_8009CE0C/D_8009CE0E (byte offsets 0 and 4), one word at
 * D_8009CE14, a 0x14-entry word array at D_8009DFB0, then a run of
 * gp-relative scalar clears.
 * era -O2 -G8, maspsx 2.21 --dont-expand-li: the small scalars resolve
 * gp-relative while the arrays stay absolute. Static absolute-symbol
 * derivation gives every D_<addr> its address. */
extern short D_8009CE0C[][2];
extern short D_8009CE0E[][2];
extern int D_8009DFB0[];

extern int D_8009CE08;
extern int D_8009CE14;
extern short D_8009CE18;
extern short D_8009CE1C;
extern short D_8009CE20;
extern short D_8009CE24;
extern short D_8009CE28;
extern short D_8009CE2C;
extern int D_8009D1D8;
extern int D_8009D1FC;
extern int D_8009D2F8;
extern int D_8009D248;
extern short D_8009D264;
extern short D_8009D1CC;

void func_8001A890(void) {
    unsigned int i;
    unsigned int j;

    D_8009CE08 = 0;
    for (i = 0; i < 2; i++) {
        D_8009CE0C[i][0] = 0;
        D_8009CE0E[i][0] = 0;
    }
    D_8009CE14 = 0;
    for (j = 0; j < 0x14; j++) {
        D_8009DFB0[j] = 0;
    }
    D_8009CE18 = 0;
    D_8009CE28 = 0;
    D_8009CE24 = 0;
    D_8009CE20 = 0;
    D_8009CE1C = 0;
    D_8009CE2C = 0;
    D_8009D1D8 = 0;
    D_8009D1FC = 0;
    D_8009D2F8 = 0;
    D_8009D248 = 0;
    D_8009D264 = 0;
    D_8009D1CC = 0;
}
