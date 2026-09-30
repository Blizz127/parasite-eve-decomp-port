extern int D_800B0CD8;
extern int D_8009D1A0;
extern int D_8009CDDC;
extern char D_800BCDE0[];
extern char D_800BCDDF[];
extern char D_800BCDC8[];

void func_80075424();
void func_80075358();
void func_800867E4();
int func_8008682C();

int func_8006A0E8(void) {
    char *base;
    int idx;
    int result;

    base = (char *)&D_800B0CD8;
    if ((*(int *)base & 0x200) == 0) {
        if (D_8009D1A0 & 0x10) {
            idx = D_8009CDDC * 0x5C;
            D_800BCDE0[idx] = 0;
            idx = D_8009CDDC * 0x5C;
            D_800BCDDF[idx] = 1;
            idx = D_8009CDDC * 0x5C;
            func_80075424(&D_800BCDC8[idx]);
            func_80075358(base + 0x114);
            func_80075358(base + 0x104);
            idx = D_8009CDDC * 0x5C;
            D_800BCDE0[idx] = 1;
            idx = D_8009CDDC * 0x5C;
            D_800BCDDF[idx] = 0;
            func_800867E4(0);
        }
    }
    result = D_8009D1A0 & 0x20;
    if (result != 0) {
        result = func_8008682C(0);
    }
    return result;
}
