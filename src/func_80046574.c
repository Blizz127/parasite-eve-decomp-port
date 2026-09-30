extern int D_8009CF18;
extern int D_8009CFB0;
extern int *func_80062A34(int, int);
extern int func_80063428(int *);
extern void func_80059534(int);
extern int func_8005968C(int);
extern void func_8004CC50(int, int);
extern void func_80062CB8(int *);
extern void func_80062F3C(int);
extern void func_800542A0(int);
extern int *func_80062CC4(void);
extern void func_8005E114(int);

void func_80046574(int window, int a1)
{
    int *list;
    int *other;
    register int *saved asm("$2");

    if (a1 == 0) {
        goto end;
    }
    list = func_80062A34(2, 7);
    if (D_8009CF18 != 0) {
        register int *base asm("$4");
        func_80059534(func_80063428(list));
        list[0x11] = -1;
        base = func_80062A34(2, 5);
        asm volatile("" : "=r"(base) : "0"(base));
        if (base[0x11] < 0) {
            goto reget;
        }
        saved = base;
        asm volatile("" : "=r"(saved) : "0"(saved));
        goto docb;
    }
    if (func_8005968C(func_80063428(list)) != 0) {
        goto armor_ok;
    }
    func_8004CC50(0x1D, 0);
    goto refresh;
armor_ok:
    {
        register int *base asm("$4");
        list[0x11] = -1;
        base = func_80062A34(2, 5);
        asm volatile("" : "=r"(base) : "0"(base));
        if (base[0x11] >= 0) {
            saved = base;
            goto docb;
        }
    }
reget:
    saved = func_80062A34(2, 6);
docb:
    func_80062CB8(saved);
    func_80062F3C(0x35);
refresh:
    func_800542A0(D_8009CF18 != 0 ? 0x1FE : 0x200);
    list = func_80062A34(2, 6);
    if (list == func_80062CC4()) {
        list[0x12] = 0;
        list[0x11] = 0;
    }
    if (D_8009CF18 != 0) {
        func_8005E114(1);
        D_8009CFB0 = 1;
    }
end:
    ;
}
