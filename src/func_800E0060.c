extern unsigned char *D_800B0E5C;
extern short D_800E21A4;
extern unsigned char *D_800E2800;

void func_800E0060(void)
{
    unsigned char *p;
    int i;

    p = D_800B0E5C - 0x14;
    D_800E2800 = p;
    i = 0;
    while (i < D_800E21A4) {
        if (*p != 0) {
            *p = 0;
            i++;
        }
        p -= 0x14;
    }
    D_800E21A4 = 0;
}
