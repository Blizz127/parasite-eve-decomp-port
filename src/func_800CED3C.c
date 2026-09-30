extern int D_800B0E18;
extern int D_800B0E1C;
extern short D_800F34E4;
extern void func_8007506C(short *a0, int a1);

void func_800CED3C(int a0)
{
    short buf[4];
    int p;

    buf[0] = 0x380;
    buf[1] = 0x100;
    buf[2] = 0x40;
    buf[3] = 0x100;
    if (a0 == 0) {
        p = D_800B0E18;
    } else {
        p = D_800B0E1C;
    }
    func_8007506C(buf, p);
    D_800F34E4 = a0;
}
