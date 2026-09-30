extern short *D_8009D2F0;
extern short *D_8009D254;
extern short D_800BCFFE;

int func_80018F74(int **a0) {
    short *s;

    s = D_8009D2F0;
    s[0x13] = (short)(**a0 >> 4);
    if (s == D_8009D254) {
        D_800BCFFE = (short)((**a0 >> 4) * 384 / 4096);
    }
    return 1;
}
