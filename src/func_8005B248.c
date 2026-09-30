extern short *D_8009D0AC;
extern int D_8009D0B0;
extern unsigned char *(*D_8009D0B4)(short);
extern int D_8009D0A0;
extern int D_8009D0A4;
extern void func_800723A4(void *, int, int, void *);
extern int func_8005B124();

void func_8005B248(void)
{
    short *p;
    short *start;
    int mask;

    p = D_8009D0AC;
    D_8009D0A0 = D_8009D0A4;
    mask = 0x1FE;
    if (p >= D_8009D0AC + D_8009D0B0) {
        return;
    }
    while (*p != 0) {
        if ((mask >> D_8009D0B4(*p)[6]) & 1) {
            break;
        }
        p++;
        if (p >= D_8009D0AC + D_8009D0B0) {
            return;
        }
    }
    if (p < D_8009D0AC + D_8009D0B0 && *p != 0) {
        start = p;
        while (*p != 0) {
            if (!((mask >> D_8009D0B4(*p)[6]) & 1)) {
                break;
            }
            p++;
            if (p >= D_8009D0AC + D_8009D0B0) {
                break;
            }
        }
        func_800723A4(start, p - start, 2, func_8005B124);
    }
}
