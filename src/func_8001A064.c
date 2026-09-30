extern unsigned int *D_8009D2F0;
extern unsigned int *D_8009D20C;

int func_8001A064(void) {
    unsigned int *s;
    unsigned int *p;

    s = D_8009D2F0;
    s[0x26] &= 0xFFEFFFFF;
    p = D_8009D20C;
    while (p != 0) {
        if (p[0x63] == (unsigned int)s) {
            p[0x63] = 0;
            p[0x26] &= 0xFF9FFFFF;
        }
        p = (unsigned int *)p[1];
    }
    return 1;
}
