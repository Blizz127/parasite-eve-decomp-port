extern unsigned char *func_80062A20(int, int);
extern int func_80063428(unsigned char *);
extern void func_80058454();
extern unsigned char *func_80062A34(int, int);
extern void func_80062CB8(unsigned char *);
extern void func_80058670();
extern void func_80062F9C();
extern void func_800512AC(int, int);
extern void func_800525EC();

int func_80048838(int a0, int a1) {
    unsigned char *p;

    if ((a1 & 0x10000) != 0) {
        p = func_80062A20(a0, 0);
        switch (func_80063428(p)) {
        case 0:
            func_80058454();
            *(int *)(p + 0x48) = 2;
            break;
        case 1:
            p = func_80062A34(2, 0xD);
            *(int *)(p + 0x48) = 0;
            *(int *)(p + 0x44) = 0;
            func_80062CB8(p);
            break;
        case 2:
            func_80058670();
            func_80062F9C();
            func_800512AC(0xA, 0);
            break;
        default:
            return 1;
        }
        func_800525EC();
    }
    return 1;
}
