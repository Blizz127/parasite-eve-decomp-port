extern int D_8009D038;
extern unsigned char D_800A1B90[];
extern unsigned char D_800A1BB0[];
extern unsigned char D_800A1D79[];
extern unsigned char D_800A1B70[];

unsigned char func_80052B2C(void)
{
    register int i asm("$4");
    register unsigned char *p asm("$5");
    int n;

    n = D_8009D038 + 1;
    D_8009D038 = n;
    if (n >= 0x209) {
        i = 0;
        p = D_800A1B90;
        do {
            *p ^= D_800A1D79[i];
            i++;
            p++;
        } while (i < 0x20);
        i = 0x20;
        p = D_800A1BB0;
        do {
            *p ^= D_800A1B70[i];
            i++;
            p++;
        } while (i < 0x209);
        D_8009D038 = 0;
    }
    return D_800A1B90[D_8009D038];
}
