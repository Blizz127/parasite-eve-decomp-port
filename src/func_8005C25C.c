extern unsigned short D_800A1E6E[];
extern short D_800C1EAC[];
extern int D_800C0E44[];
extern short D_800C0E40[];
extern unsigned int D_800A76BC[];
extern unsigned int D_800A76A4[];
extern int D_800C0DE8[];
extern int D_800C0DEC[];
extern unsigned char D_800C0DFF[];
extern unsigned char D_800C0DFD[];
extern unsigned char D_800C0DFE[];
extern short D_800C0E3C[];
extern short D_800C0E3E[];
extern int D_800A7918[];
extern int func_800614A0(void);
extern int func_800438E0(void);
extern int func_8005E884(void);
extern int func_800527B4(void);
extern int func_80064A48(void);
extern int func_80043474(int);
extern int func_8005D940(void);

void func_8005C25C(void)
{
    int i;
    int off;
    short *dst;
    int t;
    int u;

    i = 0;
    dst = D_800C1EAC;
    off = 0;
    do {
        *dst = *(unsigned short *)((char *)D_800A1E6E + off);
        off += 0x20;
        i++;
        dst++;
    } while (i < 6);
    D_800C0E44[0] = func_800614A0();
    t = func_800438E0();
    D_800C0E40[0] = t;
    D_800C0DE8[0] = D_800A76BC[0] / 60;
    D_800C0DEC[0] = D_800A76A4[0] / 60;
    D_800C0DFF[0] = func_8005E884();
    u = func_800527B4();
    D_800C0DFD[0] = u | (func_80064A48() << 2);
    D_800C0DFE[0] = 0;
    D_800C0E3C[0] = func_80043474(D_800A7918[0]);
    D_800C0E3E[0] = func_8005D940();
}
