extern int D_8009D0C4;
extern unsigned char *D_8009D0C0;

int func_8005BF08(void)
{
    int n;
    unsigned char *p;
    unsigned int c;

    n = D_8009D0C4;
    p = D_8009D0C0;
    while (n != 0 && (c = *p) != 0xFF) {
        n -= (c < 0xFA);
        p++;
    }
    return n;
}
