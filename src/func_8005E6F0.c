extern int D_8009D120;
extern int D_8009D108;
extern int D_8009CDDC[];
extern unsigned char D_800A2180[];
extern int D_800A21F0[];
extern int D_800A21F4[];
extern unsigned char *D_8009D0FC;
extern int D_8009D100;
extern int D_8009D104;
extern int D_8009D118;
extern int D_8009D11C;
extern void func_800752AC(int, int);

void func_8005E6F0(void)
{
    int idx;
    int off;
    int a;
    int b;

    if (D_8009D120 != 0) {
        idx = (D_8009D108 == 0);
    } else {
        idx = D_8009CDDC[0];
    }
    off = idx * 0x78;
    D_8009D108 = idx;
    a = *(int *)((char *)D_800A21F4 + off);
    b = *(int *)((char *)D_800A21F0 + off);
    D_8009D0FC = D_800A2180 + off;
    D_8009D104 = a;
    D_8009D100 = a;
    D_8009D118 = b;
    D_8009D11C = b + 4;
    if (D_8009D120 != 0) {
        func_800752AC(b, 0x1000);
    }
}
