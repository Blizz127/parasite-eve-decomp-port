extern unsigned char D_800923F8[];
extern unsigned char D_80092458[];
extern unsigned char D_80092410[];
extern unsigned char D_80092468[];
extern unsigned char D_800C1EB8[];
extern unsigned char *D_8009D0AC;
extern int D_8009D0B0;
extern void *D_8009D0B4;
extern unsigned char *D_8009D0B8;
extern unsigned char *D_8009D0BC;
extern void func_800532B4();
extern void func_8005AFFC();
extern void func_800723A4(unsigned char *, int, int, void *);
extern void func_8005B248(void);
extern void func_8005B3A4(void);
extern void func_8005B500(int, int);
extern void func_80058C4C(int);

void func_8005B71C(int a0)
{
    if (a0 != 0) {
        D_8009D0B8 = D_800923F8;
        D_8009D0BC = D_80092458;
    } else {
        D_8009D0B8 = D_80092410;
        D_8009D0BC = D_80092468;
    }
    D_8009D0B0 = 0x64;
    D_8009D0AC = D_800C1EB8;
    D_8009D0B4 = func_800532B4;
    func_800723A4(D_800C1EB8, 0x64, 2, func_8005AFFC);
    func_8005B248();
    func_8005B3A4();
    func_8005B500(2, a0);
    func_80058C4C(0xF400);
}
