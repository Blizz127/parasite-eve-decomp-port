extern short D_80194160;
extern short D_80194162;
extern short D_80194164;

short *func_80193FF0(int a0, int a1, int a2, int a3)
{
    if (a0 == 1) {
        D_80194160 = (short)a1;
        D_80194162 = (short)a2;
        D_80194164 = (short)a3;
    }
    return &D_80194160;
}
