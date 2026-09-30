extern int D_8009D0C4;
extern unsigned char *D_8009D0C0;

void func_8005BD10(int a0)
{
    register unsigned char *p asm("$3");
    unsigned int c;
    int i;

    p = D_8009D0C0;
    i = 0;
    while (i < D_8009D0C4 && (c = *p) != 0xFF) {
        i += (c < 0xFA);
        p++;
    }
    if (i < D_8009D0C4) {
        p[0] = a0;
        p[1] = 0xFF;
    } else {
        if (p[-2] >= 0xFA) {
            p--;
            p[0] = 0xFF;
        }
        p[-1] = a0;
    }
}
