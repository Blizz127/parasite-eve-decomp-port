extern unsigned char *D_8009D048;
extern int D_8009D050;
extern int *D_8009D058;
extern int D_8009D064;
extern unsigned char D_800C0E48;
extern int D_8009D05C;
extern int func_80052F70(void);
extern void func_80057D30(int);

int func_8005409C(int key)
{
    short *p;
    int idx;

    D_8009D048 = &D_800C0E48;
    D_8009D050 = func_80052F70();
    p = (short *)D_8009D048;
    D_8009D058 = &D_8009D05C;
    D_8009D064 = 2;
    idx = -1;
    while (p < (short *)D_8009D048 + D_8009D050 && *p != key) {
        p++;
    }
    if (p < (short *)D_8009D048 + D_8009D050) {
        idx = p - (short *)D_8009D048;
    } else {
        idx = -1;
    }
    if (idx >= 0) {
        func_80057D30(idx);
        idx = 0;
    }
    return idx;
}
