extern int D_8009D0C4;
extern unsigned char *D_8009D0C0;

int func_8005BD94(void)
{
    register unsigned char *p asm("$4");
    unsigned int c;
    int n;
    register int i asm("$3");
    int r;

    n = D_8009D0C4;
    p = D_8009D0C0;
    i = 0;
    while (i < n && (c = *p) != 0xFF) {
        i += (c < 0xFA);
        p++;
    }
    r = (0 < i);
    p--;
    if (r != 0) {
        *p = 0xFF;
        p--;
        if (p >= D_8009D0C0 && *p >= 0xFA) {
            *p = 0xFF;
        }
    }
    return r;
}
