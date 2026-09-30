extern int D_800E27EC;
extern char D_800E1CC8;
extern char D_800E2224;
extern void func_800CF3AC(char *a0, int *a1, int a2);
extern void func_800DB25C(char *a0, int a1, int a2, int a3, int a4, int a5, int a6, int *a7);

int func_800DB5F4(int a0, short *a1)
{
    int buf[2];
    int v;

    switch (a0) {
    case 1:
        if (D_800E27EC < 8) {
            break;
        }
        return 1;
    case 2:
        v = 0x80 - (D_800E27EC << 4);
        func_800CF3AC(&D_800E1CC8, buf, a1[5]);
        func_800DB25C(&D_800E2224, a1[0], a1[1], a1[2], a1[3], 1, v, buf);
        break;
    default:
        return 0;
    }
    return 0;
}
