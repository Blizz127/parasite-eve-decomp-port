extern int D_8009CFE0;
extern int D_800C0E44[];
extern unsigned char *func_80062A20(int, int);
extern int func_80063428(unsigned char *);
extern int func_800614A0(void);
extern void func_800614AC(int);
extern void func_8005267C(void);
extern void func_80062F1C(int);
extern void func_80062CB8(unsigned char *);
extern void func_800525EC(void);
extern void func_80052634(void);

int func_8004B394(int page, unsigned int ev)
{
    int sh;
    int col;
    int c;
    int m;
    int x;
    unsigned char *o;
    int s;

    sh = func_80063428(func_80062A20(page, 0)) << 3;
    col = func_800614A0();
    if (ev & 0x1000) {
        m = col & ~(0xFF << sh);
        c = ((col >> sh) & 0xFF) + 2;
        x = ((c > 0xE8) ? 0xE8 : c) << sh;
    } else if (ev & 0x4000) {
        m = col & ~(0xFF << sh);
        c = ((col >> sh) & 0xFF) - 2;
        x = ((c < 0x20) ? 0x20 : c) << sh;
    } else {
        goto other;
    }
    func_800614AC(m | x);
    func_8005267C();
    return 1;
other:
    if (ev & 0x10000) {
        o = func_80062A20(page, 1);
        s = func_80063428(o);
        if (s == 0) {
            goto keep;
        }
        if (s == 1) {
            func_800614AC(0x404040);
        keep:
            D_800C0E44[0] = func_800614A0();
            func_80062F1C(page);
        } else {
            *(int *)(o + 0x44) = 0;
            *(int *)(o + 0x48) = 0;
            func_80062CB8(o);
            *(int *)(func_80062A20(page, 0) + 0x44) = -1;
        }
        func_800525EC();
        return 1;
    }
    if (ev & 0x40) {
        func_80062F1C(page);
        func_800614AC(D_8009CFE0);
        func_80052634();
    }
    return 1;
}
