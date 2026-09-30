extern unsigned int D_800C0D90;
extern unsigned int D_800C0DA4;
extern unsigned short D_800C0DA2;
extern unsigned short D_800C0DA0;
extern unsigned short D_8009D2B6;
extern void func_80085F74(void *a0);

void func_8008D7D0(void) {
    unsigned int *p;
    unsigned short v;

    p = &D_800C0D90;
    v = D_8009D2B6;
    *p = 0x1C0;
    D_800C0DA4 = 0;
    D_800C0DA2 = v;
    D_800C0DA0 = v;
    func_80085F74(p);
}
