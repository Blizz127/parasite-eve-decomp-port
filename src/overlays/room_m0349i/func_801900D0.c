extern short D_80190194;
extern short D_80190196;
extern short D_80190198;
extern int D_8019019C;
extern int D_801901A0;
extern int D_801901A4;

short *func_801900D0(int a0, int a1, int a2, int a3)
{
    if (a0 == 1) {
        D_80190194 = (short)a1;
        D_80190196 = (short)a2;
        D_80190198 = (short)a3;
    }
    if (a0 == 2) {
        D_8019019C = a1;
        D_801901A4 = a2;
    } else {
        if (a1 < 2) {
            a1 = 2;
        }
        D_801901A0 = a1;
    }
    return &D_80190194;
}
