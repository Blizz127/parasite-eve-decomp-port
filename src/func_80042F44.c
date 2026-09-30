extern int D_8009CED8;
extern int D_8009CEDC;
extern int D_800B0E50;

extern void func_800750CC(short *, int);
extern void func_80042D40();

void func_80042F44(void) {
    short buf[4];
    int r;

    switch (D_8009CED8) {
    case 1:
        r = 2;
        break;
    case 2:
        r = 3;
        break;
    case 3:
        buf[0] = 0;
        buf[1] = 0x1E0;
        buf[2] = 0x100;
        buf[3] = D_8009CEDC;
        func_800750CC(buf, D_800B0E50);
        r = 4;
        break;
    case 5:
        D_8009CED8 = 6;
        func_80042D40();
        return;
    case 4:
    case 6:
        r = 5;
        break;
    case 0:
    case 7:
    default:
        return;
    }
    D_8009CED8 = r;
}
