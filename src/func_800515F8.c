extern short **D_8009D254;
extern unsigned char *D_8009D014;
extern unsigned char D_800A1AA0[];

int func_800515F8(int *a0)
{
    short *rec;
    unsigned char *p;
    unsigned char *end;
    int acc;

    acc = 0;
    if (D_8009D254 != 0 && *D_8009D254 != 0) {
        rec = *D_8009D254;
        p = D_800A1AA0;
        end = D_8009D014;
        acc = rec[5];
        while (p < end) {
            if (*(int *)p == 1) {
                acc -= *(int *)(p + 8);
            }
            p += 0x24;
        }
        if (a0 != 0) {
            *a0 = rec[0x15];
        }
    }
    return acc;
}
