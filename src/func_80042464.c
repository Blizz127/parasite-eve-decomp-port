extern short *D_800A1854;
extern int D_800A1858;

int func_80042464(void) {
    int r;
    int v;

    if (D_800A1854 != 0) {
        v = (D_800A1858 - D_800A1854[0xA] + 0x400) >> 10;
        if (v < 9) {
            r = v;
        } else {
            r = 8;
        }
    } else {
        r = 0;
    }
    return r;
}
