/* VRAM 0x8004620C / file 0x36A0C / size 0x128. */
int func_80062A20();
int func_80062A34();
int func_80047FE0();
int func_80062F3C();
int func_80062F1C();
int func_800439D8();
int func_80052634();
int func_80062CB8();
int func_8005267C();

int func_8004620C(int arg0, int arg1) {
    int s0;
    int s1;
    int s3;
    int v0;

    s3 = 0;
    s0 = func_80062A20(arg0, 0);
    s1 = func_80062A34(2, 0xB);
    if (s1 != 0) {
        s3 = func_80047FE0(s0, s1, arg1);
    }
    v0 = arg1 & 0x1000;
    if (s3 != 0) goto check;
    v0 = arg1 & 0x40;
    if (v0 != 0) {
        if (s1 == 0) {
            func_80062F3C(7);
            func_80062F1C(arg0);
            func_80062F3C(5);
            func_800439D8();
            func_80052634();
        }
        return 1;
    }
    v0 = arg1 & 0x1000;
check:
    if (v0 != 0) {
        *(int *)(s0 + 0x44) = -1;
        if (s1 == 0) {
            s0 = func_80062A34(2, 5);
        } else {
            s0 = func_80062A34(2, 0x1B);
        }
        if (s0 != 0) {
            *(int *)(s0 + 0x44) = 0;
            *(int *)(s0 + 0x48) = *(int *)(s0 + 0x58) - 1;
            func_80062CB8(s0);
        }
        func_8005267C();
    }
    return 1;
}
