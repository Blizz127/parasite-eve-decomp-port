extern unsigned char *D_8009D048;
extern int D_8009D050;
extern int *D_8009D058;
extern int D_8009D064;
extern unsigned char D_800C0E48;
extern int D_8009D05C;
extern int func_80052F70(void);

int func_8005401C(void)
{
    short *p;
    short *end;
    int n;
    int count;

    count = 0;
    D_8009D048 = &D_800C0E48;
    n = func_80052F70();
    p = (short *)D_8009D048;
    D_8009D058 = &D_8009D05C;
    D_8009D050 = n;
    D_8009D064 = 2;
    end = p + n;
    while (p < end) {
        count += (*p != 0);
        p++;
    }
    return count;
}
