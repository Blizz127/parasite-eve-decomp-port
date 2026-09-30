extern int D_800E27EC;
extern char D_800E17E0;
extern int func_80071A54(void);
extern void func_800CF3AC(char *a0, int *a1, int a2);
extern void func_800D1DEC(short *a0, int *a1, int a2, int a3);

int func_800D70C0(int a0, short *a1)
{
    short buf[4];
    int b[2];
    int r;
    unsigned short w;

    switch (a0) {
    case 1:
        r = func_80071A54();
        w = a1[0] - 3;
        a1[0] = w + (r & 7);
        r = func_80071A54();
        w = a1[1] - 7;
        a1[1] = w + (r & 3);
        r = func_80071A54();
        w = a1[2] - 3;
        a1[2] = w + (r & 7);
        if (D_800E27EC < 0x12) {
            break;
        }
        return 1;
    case 2:
        buf[0] = a1[0];
        buf[1] = a1[1];
        buf[2] = a1[2];
        func_800CF3AC(&D_800E17E0, b, D_800E27EC);
        func_800D1DEC(buf, b, 0x80, 1);
        break;
    default:
        return 0;
    }
    return 0;
}
