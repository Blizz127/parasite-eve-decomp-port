extern int D_800E27EC;
extern char D_800E1AC8;
extern int func_80071A54(void);
extern void func_800CF3AC(char *a0, int *a1, int a2);
extern void func_800D1DEC(short *a0, int *a1, int a2, int a3);

int func_800D9554(int a0, short *a1)
{
    short buf[4];
    int b[2];
    int r;
    unsigned short w;

    switch (a0) {
    case 1:
        r = func_80071A54();
        w = a1[0] - 1;
        a1[0] = w + (a1[3] + (r & 3));
        r = func_80071A54();
        w = a1[1] - 1;
        a1[1] = w + (a1[4] + (r & 3));
        a1[2] = a1[2] + a1[5];
        a1[3] = a1[3] * 31 / 32;
        a1[5] = a1[5] * 31 / 32;
        if (D_800E27EC < 0x13) {
            a1[4]--;
        } else {
            a1[4]++;
        }
        if (D_800E27EC < 0x28) {
            break;
        }
        return 1;
    case 2:
        buf[0] = a1[0];
        buf[1] = a1[1];
        buf[2] = a1[2];
        if (D_800E27EC < 0x13) {
            func_800CF3AC(&D_800E1AC8, b, D_800E27EC);
        } else if (!(func_80071A54() & 3)) {
            b[0] = 0xC8C8;
        } else {
            b[0] = 0;
        }
        func_800D1DEC(buf, b, 0x80, 1);
        break;
    default:
        return 0;
    }
    return 0;
}
