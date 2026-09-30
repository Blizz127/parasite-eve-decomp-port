extern int D_8009D04C;
extern int D_8009D054;
extern unsigned char *D_8009D048;
extern int D_8009D050;
extern int *D_8009D058;
extern int D_8009D064;
extern unsigned char D_800C0E48;
extern int D_8009D05C;
extern int func_80052F70(void);

void func_80052EC0(void)
{
    D_8009D04C = 0;
    D_8009D054 = 0;
    D_8009D048 = &D_800C0E48;
    D_8009D050 = func_80052F70();
    D_8009D058 = &D_8009D05C;
    D_8009D064 = 2;
}
