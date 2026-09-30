extern unsigned char D_800A2090[];
extern unsigned char D_800A2174[];
extern unsigned char *D_8009D0DC;
extern int D_8009D0E0;
extern int D_8009D0E4;
extern int D_8009D0E8;
extern int D_8009D0EC;
extern int D_8009D0F0;

void func_8005DE88(void)
{
    unsigned char *p;
    unsigned char *end;
    unsigned char *q;

    p = D_800A2090;
    end = D_800A2090 + 0xF0;
    while (p < end) {
        *(unsigned char **)p = p + 0xC;
        p = p + 0xC;
    }
    q = D_800A2174;
    *(unsigned char **)q = 0;
    D_8009D0DC = q - 0xE4;
    D_8009D0E4 = 0;
    D_8009D0E0 = 0;
    D_8009D0E8 = 0;
    D_8009D0EC = 0;
    D_8009D0F0 = 0;
}
