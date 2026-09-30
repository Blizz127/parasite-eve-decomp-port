extern int D_800A1B30;
extern unsigned short *func_8005DBAC(int);

int func_80051DF8(int a0)
{
    return (D_800A1B30 + 0x14) * *func_8005DBAC(a0) / 20;
}
