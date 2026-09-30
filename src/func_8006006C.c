extern int D_8009D124;
extern int D_8009D128;
extern int D_8009D13C;
extern int D_8009D140;
extern int D_8009D144;
extern void func_8005F874(int);
extern void func_8005EB64(int);

void func_8006006C(int t, int mode)
{
    int x;
    int y;

    if (t > 0x57E40) {
        t = 0x57E3F;
    }
    D_8009D13C = mode ? 0x3A1C : 0x395D;
    D_8009D140 = mode ? 0xCC : 0x84;
    D_8009D144 = 0xA4;
    func_8005F874(t / 36000);
    x = D_8009D124;
    y = D_8009D128;
    D_8009D124 = x + 5;
    D_8009D128 = y;
    func_8005F874(t / 3600);
    x = D_8009D124;
    y = D_8009D128;
    D_8009D124 = x + 5;
    D_8009D128 = y;
    if (mode != 0) {
        func_8005EB64(0x9B);
        x = D_8009D124;
        y = D_8009D128;
        D_8009D124 = x + 3;
        D_8009D128 = y;
    } else {
        func_8005EB64(0x4F);
        x = D_8009D124;
        y = D_8009D128;
        D_8009D124 = x + 5;
        D_8009D128 = y;
    }
    func_8005F874((t / 600) % 6);
    x = D_8009D124;
    y = D_8009D128;
    D_8009D124 = x + 5;
    D_8009D128 = y;
    func_8005F874(t / 60);
    x = D_8009D124;
    y = D_8009D128;
    D_8009D124 = x + 5;
    D_8009D128 = y;
    if (mode != 0) {
        func_8005EB64(0x9B);
        x = D_8009D124;
        y = D_8009D128;
        D_8009D124 = x + 3;
        D_8009D128 = y;
    } else {
        func_8005EB64(0x4F);
        x = D_8009D124;
        y = D_8009D128;
        D_8009D124 = x + 5;
        D_8009D128 = y;
    }
    func_8005F874((t / 10) % 6);
    x = D_8009D124;
    y = D_8009D128;
    D_8009D124 = x + 5;
    D_8009D128 = y;
    func_8005F874(t);
    D_8009D13C = 0x395D;
    D_8009D140 = 0x84;
    D_8009D144 = 0xA4;
}
