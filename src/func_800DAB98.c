extern int D_800E27EC;
extern char D_800E1C04;
extern int func_80077DC4(int a0);
extern void func_800CF3AC(char *a0, int *a1, int a2);
extern void func_800D27FC(int a0, int a1, int *a2, int a3, int a4);

int func_800DAB98(int a0, short *a1)
{
    int buf[2];
    int ang;
    int v;

    switch (a0) {
    case 1:
        ang = (D_800E27EC << 10) / a1[3];
        v = func_80077DC4(ang);
        a1[0] = a1[1] * v / 4096;
        a1[2] = a1[2] + 0x10;
        if (D_800E27EC < a1[3]) {
            break;
        }
        return 1;
    case 2:
        func_800CF3AC(&D_800E1C04, buf, D_800E27EC);
        func_800D27FC(a1[0], a1[2], buf, 0x80, 1);
        break;
    default:
        return 0;
    }
    return 0;
}
