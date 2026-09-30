typedef struct Task {
    unsigned char pad0[0x18];
    int f18;
    int f1C;
    unsigned char pad20[4];
    int f24;
    unsigned char pad28[4];
    void (*f2C)();
    void (*f30)();
    unsigned char pad34[4];
    int f38;
    unsigned char pad3C[0x1C];
    int f58;
    unsigned char pad5C[0x30];
    void (*f8C)();
} Task;

extern int D_8009CF24;
extern int D_8009CF28;
extern int D_8009CFBC;
extern int D_8009CF18;
extern int D_8009CF0C;
extern int D_8009CF2C;
extern int D_8009CF14;
extern int D_800A1888[];
extern int D_800A188C;
extern int D_800A1890;
extern int D_800A1894;
extern Task *func_80062CC4(void);
extern int func_80063428(Task *);
extern int func_80059F08(int);
extern unsigned char *func_8005332C(int);
extern int func_80054F58(int, int);
extern int func_8005415C(int);
extern Task *func_80062D2C(int, Task *, int, int);
extern Task *func_8006322C(int, Task *, Task *);
extern void func_80062CB8(Task *);
extern void func_80047714();
extern void func_8004778C();
extern void func_80050038();
extern void func_80050020();
extern void func_8004790C();
extern void func_80047A30();
extern void func_8004F950();
extern void func_800480AC(void);
extern void func_800525EC(void);
extern void func_800526C4(void);
extern void func_8004CC50(int, int);

void func_80048254(void)
{
    int side;
    int id;
    int r;
    int n;
    int i;
    int *p;
    Task *e;
    Task *f;
    Task *e2;
    Task *f2;

    side = 0;
    id = -1;
    switch (func_80062CC4()->f24) {
    case 6:
        id = func_80063428(func_80062CC4());
    case 0x1B:
        side = 0;
        break;
    case 0xB:
        id = func_80063428(func_80062CC4());
    case 0x1C:
        side = 1;
        break;
    }
    if ((func_8005332C(func_80059F08(side)) + id)[0x15] != 0) {
        D_8009CF24 = side;
        D_8009CF28 = id;
        r = func_80054F58(side, id);
        D_8009CFBC = r == 5;
        if (r == 1 || r == 2 || r == 5) {
            D_8009CF18 = func_8005415C(func_80059F08(side)) != 9;
            if ((D_800A1888[0] > 0) + (D_800A188C > 0) + (D_800A1890 > 0) + (D_800A1894 > 0) >= 2) {
                e = func_80062D2C(0x2C, func_80062CC4(), 0, 1);
                f = func_8006322C(0x2C, e, e);
                e->f30 = func_80047714;
                e->f2C = func_8004778C;
                f->f30 = func_80050038;
                f->f8C = func_80050020;
                if (D_8009CF0C != 0) {
                    e->f38 += 0x10;
                    f->f38 = 2;
                    f->f58 = 2;
                    f->f18 += 0x1C;
                }
                func_80062CB8(f);
            } else {
                for (i = 0; i < 4 && D_800A1888[i] == 0; i++) {
                }
                if (i < 4) {
                    D_8009CF2C = i;
                } else {
                    D_8009CF2C = 0;
                }
                i = (func_8005332C(func_80059F08(side)) + id)[0x15] & 0x1F;
                if (r == 1 || (!(D_8009CF2C & 1) ? r == 2 : (r == 5 && (unsigned int)(i - 8) >= 3))) {
                    e2 = func_80062D2C(4, func_80062CC4(), 0, 1);
                    f2 = func_8006322C(4, e2, e2);
                    e2->f30 = func_8004790C;
                    e2->f2C = func_80047A30;
                    f2->f30 = func_8004F950;
                    D_8009CF14 = 5;
                    func_80062CB8(f2);
                    if (D_8009CF2C & 1) {
                        e2->f38 -= 0x1C;
                        f2->f1C -= 0x1C;
                    }
                } else {
                    func_800480AC();
                }
            }
            func_800525EC();
        } else {
            switch (r) {
            case 0:
                func_800480AC();
                break;
            case 4:
                func_8004CC50(0x63, 0);
                break;
            default:
                func_8004CC50(7, 0);
                break;
            }
        }
    } else {
        func_800526C4();
    }
}
