extern int D_800E27EC;
extern char D_800E1C2C;
extern char D_800E221C;
extern int func_80077CF4(int a0);
extern void func_800CF3AC(char *a0, int *a1, int a2);
extern void func_800D0E88(char *a0, short *a1, int a2, int a3, int *a4, int a5, int a6, int a7, int a8);

int func_800DAF8C(int a0, short *a1)
{
    int buf[4];
    register int A asm("$5");
    int T;
    register int v asm("$2");

    switch (a0) {
    case 1:
        A = a1[6];
        T = D_800E27EC;
        a1[1] = a1[1] + a1[7];
        a1[2] = a1[2] + 8;
        v = A - A * T / 64;
        a1[0] = v;
        a1[4] = func_80077CF4(T * 32) / 64;
        a1[5] = 0x640 - D_800E27EC * 1000 / 64;
        if (D_800E27EC < 0x40) {
            break;
        }
        return 1;
    case 2:
        func_800CF3AC(&D_800E1C2C, buf, D_800E27EC);
        func_800D0E88(&D_800E221C, a1, a1[5], a1[4], buf, 0, 0, 0x80, 1);
        break;
    default:
        return 0;
    }
    return 0;
}
