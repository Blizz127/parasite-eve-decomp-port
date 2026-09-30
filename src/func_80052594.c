extern unsigned char D_80091694[];
extern unsigned char D_8009169D;

void func_80052594(unsigned char *a0)
{
    register unsigned char *d asm("$5");
    register unsigned char *lp asm("$3");
    unsigned char *end;
    unsigned char *bp;
    unsigned char c;

    register int n asm("$2");

    d = D_80091694;
    end = d + 8;
    while (d < end) {
        c = *a0;
        if (c == 0xFF) { break; }
        *d = c;
        d++;
        a0++;
    }
    lp = &D_8009169D;
    bp = lp;
    bp -= 9;
    n = d - bp;
    *lp = n;
}
