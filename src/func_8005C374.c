extern unsigned short D_800C1EAC[];
extern unsigned short D_800A1E6E[];
extern int D_800C0E44;
extern short D_800C0E40;
extern int D_800C0DE8;
extern int D_800C0DEC;
extern int D_800A76BC[];
extern int D_800A76A4[];
extern signed char D_800C0DFF;
extern unsigned char D_800C0DFD;
extern int D_8009D218;
extern void func_800614AC(int);
extern void func_800438C0(int);
extern int func_8005E884(void);
extern void func_8005E850(int, int);
extern void func_80052790(int);
extern void func_800649D0(int);
extern void func_80052594(unsigned char *);

void func_8005C374(void)
{
    int i;
    int off;
    unsigned short *src;
    unsigned char *base;
    unsigned char *p;

    i = 0;
    src = D_800C1EAC;
    off = 0;
    do {
        *(unsigned short *)((char *)D_800A1E6E + off) = *src;
        src++;
        i++;
        off += 0x20;
    } while (i < 6);
    base = (unsigned char *)&D_800C0E44;
    asm volatile("" : "=r"(base) : "0"(base));
    func_800614AC(*(int *)base);
    func_800438C0(D_800C0E40);
    D_800A76BC[0] = D_800C0DE8 * 60;
    D_800A76A4[0] = D_800C0DEC * 60;
    func_8005E850(0, D_800C0DFF - func_8005E884());
    func_80052790(D_800C0DFD & 3);
    func_800649D0((D_800C0DFD >> 2) & 1);
    p = base - 0x64;
    if (D_8009D218 != 0) {
        p = base - 0x54;
    }
    func_80052594(p);
}
