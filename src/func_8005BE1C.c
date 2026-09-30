extern unsigned char *D_8009D0C0;
extern int D_8009D0C4;
extern unsigned char *D_8009D0C8;
extern unsigned char *func_8005DC9C(int);
extern unsigned char *func_8005DC4C(int);

void func_8005BE1C(void)
{
    unsigned char *q;
    unsigned char *src;
    unsigned char *dst;
    unsigned char *d0;
    unsigned char *r;

    q = D_8009D0C0;
    if (q < D_8009D0C0 + D_8009D0C4) {
        do {
            *q = 0xFF;
            q++;
        } while (q < D_8009D0C0 + D_8009D0C4);
    }
    d0 = D_8009D0C0;
    if (D_8009D0C8 != 0) {
        r = func_8005DC9C(D_8009D0C8[4] - 1);
    } else {
        r = func_8005DC4C(0x1E);
    }
    dst = d0;
    src = r;
    while ((*dst++ = *src++) != 0xFF) {
        ;
    }
}
