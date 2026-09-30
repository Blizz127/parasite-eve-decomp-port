extern int D_8009D028;
extern int D_8009D02C;
extern unsigned char *D_8009D048;
extern int D_8009D050;
extern int *D_8009D058;
extern int D_8009D064;
extern unsigned char D_800C0E48;
extern int D_8009D05C;
extern int func_80033A20(void);
extern void func_800339A0(int);
extern void func_80051510(void);
extern int func_80052F70(void);
extern void func_800438EC(void);
extern void func_800525EC(void);

void func_8005C174(int a0)
{
    D_8009D028 = a0;
    D_8009D02C = func_80033A20() & 0xFF;
    func_800339A0(0);
    func_80051510();
    D_8009D048 = &D_800C0E48;
    D_8009D050 = func_80052F70();
    D_8009D058 = &D_8009D05C;
    D_8009D064 = 2;
    func_800438EC();
    func_800525EC();
}
