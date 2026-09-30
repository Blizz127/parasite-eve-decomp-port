extern unsigned char D_800A1B50[];
extern int D_8009D218;
extern unsigned char *func_8005DC4C(int);
extern unsigned char *func_8005DC9C(int);

unsigned char *func_80054A88(int a0, int a1)
{
    if (a0 == 0) {
        {
            unsigned char *s;
            unsigned char *t;
            register unsigned char *d asm("$5");
            t = func_8005DC4C(0x13);
            d = D_800A1B50;
            s = t;
            while ((*d++ = *s++) != 0xFF) {
                ;
            }
        }
    } else if (a1 >= 2) {
        {
            unsigned char *s;
            unsigned char *t;
            register unsigned char *d asm("$5");
            t = func_8005DC4C(0x12);
            d = D_800A1B50;
            s = t;
            while ((*d++ = *s++) != 0xFF) {
                ;
            }
        }
    } else if (D_8009D218 == 0) {
        {
            unsigned char *s;
            unsigned char *t;
            register unsigned char *d asm("$5");
            t = func_8005DC9C(a0 - 1);
            d = D_800A1B50;
            s = t;
            while ((*d++ = *s++) != 0xFF) {
                ;
            }
        }
        {
            unsigned char *s;
            unsigned char *t;
            register unsigned char *d asm("$4");
            t = func_8005DC4C(a1 + 0x10);
            d = D_800A1B50;
            s = t;
            while (*d++ != 0xFF) {
                ;
            }
            d--;
            while ((*d++ = *s++) != 0xFF) {
                ;
            }
        }
    } else {
        {
            unsigned char *s;
            unsigned char *t;
            register unsigned char *d asm("$5");
            t = func_8005DC4C(a1 + 0x10);
            d = D_800A1B50;
            s = t;
            while ((*d++ = *s++) != 0xFF) {
                ;
            }
        }
        {
            unsigned char *s;
            unsigned char *t;
            register unsigned char *d asm("$4");
            t = func_8005DC9C(a0 - 1);
            d = D_800A1B50;
            s = t;
            while (*d++ != 0xFF) {
                ;
            }
            d--;
            while ((*d++ = *s++) != 0xFF) {
                ;
            }
        }
    }
    return D_800A1B50;
}
