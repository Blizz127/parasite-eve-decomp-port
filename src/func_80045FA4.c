typedef struct W {
    unsigned char pad0[0x18];
    int f18;
    int f1C;
} W;

extern int D_8009CF0C;
extern int D_800A1888[];
extern W *func_80062A34(int, int);
extern int func_800631C0(W *);
extern int func_80063428(W *);
extern int func_800556E8(int);
extern int func_80059F08(int);
extern unsigned char *func_8005332C(int);
extern void func_80063158(W *, int, int);
extern int func_80054288(void);
extern void func_8005E8A4(int, int);
extern void func_8005F5B8(int);
extern void func_8005FA3C(int);
extern void func_8005FB74(int);

void func_80045FA4(W *self)
{
    W *w;
    int t;
    unsigned char *rec;
    int base;
    int h;
    int n;
    int y;
    int x;
    int z;
    int u;
    int dy;

    if (func_800631C0(func_80062A34(1, 7)) == 0) {
        w = func_80062A34(1, 6);
        if (w == 0) {
            w = func_80062A34(1, 11);
        }
        if (func_80062A34(1, 11) == 0 && func_80063428(func_80062A34(2, 7)) >= 0) {
            t = func_800556E8(func_80063428(func_80062A34(2, 7)));
        } else {
            t = func_80059F08(0);
        }
        rec = func_8005332C(t);
        x = w->f18 - self->f18;
        z = ((rec[20] + 1) >> 1) * 16;
        y = self->f1C - 108;
        func_80063158(self, x, z - y);
    } else {
        base = D_8009CF0C ? 56 : 36;
        h = self->f1C;
        if (func_80054288() < 9) {
            dy = base + func_80054288() * 16 - h;
        } else {
            u = h - 128;
            dy = base - u;
        }
        func_80063158(self, 0, dy);
    }
    func_8005E8A4(4, 4);
    func_8005F5B8(106);
    func_8005E8A4(60, 0);
    func_8005F5B8(107);
    n = D_800A1888[0] + D_800A1888[2];
    if (n < 100 && D_8009CF0C == 0) {
        func_8005E8A4(-15, 3);
        func_8005FA3C(n);
    } else {
        func_8005E8A4(-20, 3);
        func_8005FB74(n >= 1000 ? 999 : n);
    }
    n = D_800A1888[1] + D_800A1888[3];
    if (n < 100 && D_8009CF0C == 0) {
        func_8005E8A4(50, 0);
        func_8005FA3C(n);
    } else {
        func_8005E8A4(45, 0);
        func_8005FB74(n >= 1000 ? 999 : n);
    }
}
