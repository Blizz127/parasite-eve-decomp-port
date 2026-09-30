extern unsigned int *D_8009D2F0;
extern unsigned int *D_8009D20C;

int func_80019FE0(void) {
    register unsigned int *p asm("$4");
    register unsigned int *s asm("$5");

    s = D_8009D2F0;
    s[0x63] = 0;
    s[0x26] &= 0xFF9FFFFF;
    p = D_8009D20C;
    while (p != 0) {
        if (p != s) {
            if (p[0x63] == s[0x63]) {
                return 1;
            }
        }
        p = (unsigned int *)p[1];
    }
    p = (unsigned int *)D_8009D2F0[0x63];
    p[0x26] &= 0xFFEFFFFF;
    return 1;
}
