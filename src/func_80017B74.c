extern unsigned int *D_8009D2F0;
extern unsigned int *D_8009D300;
extern int D_8009CE00;

int func_80017B74(void) {
    unsigned short *s;

    D_8009D300[4] = 1;
    s = (unsigned short *)D_8009D2F0;
    if (s[0xB] == s[9]) {
        return 0;
    }
    D_8009CE00 -= 8;
    return 0;
}
