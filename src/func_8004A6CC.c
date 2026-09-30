typedef struct Stats {
    unsigned char pad0[7];
    unsigned char b[3];
    unsigned char padA[4];
    short h[3];
} Stats;

extern Stats D_800A1A00;
extern int D_8009CFAC;
extern int D_8009CFD0;
extern int D_8009CFD8;
extern int D_8009CF18;
extern int D_8009CFDC;
extern void func_8005E8A4(int, int);
extern void func_8005EB64(int);
extern void func_8006055C(int);
extern void func_80060590(int);
extern void func_8005FDF0(int);
extern void func_8005FF28(int);
extern void func_8005B91C(int, int, int *, int);
extern void func_800605F8(int);
extern void func_8005BA78(int, int, int *, int *);
extern void func_8006062C(int, int);

void func_8004A6CC(void)
{
    int v;
    int buf[3];

    func_8005E8A4(20, 21);
    func_8005EB64(D_8009CFAC + 77);
    func_8005E8A4(-14, -17);
    func_8005EB64(147);
    if (D_8009CFD0 < 3) {
        func_8005E8A4(72, 0);
        func_8006055C(D_8009CFD8);
        func_8005E8A4(-117, 30);
        func_8005EB64(D_8009CF18 ? D_8009CFD0 + 124 : D_8009CFD0 + 127);
        func_8005E8A4(30, 0);
        switch (D_8009CFD0) {
        case 0:
            v = D_800A1A00.b[0] + D_800A1A00.h[0];
            if (v >= 1000) {
                v = 999;
            }
            func_80060590(v);
            func_8005E8A4(4, 2);
            func_8005FDF0(D_800A1A00.b[0]);
            func_8005E8A4(5, 0);
            func_8005FF28(D_800A1A00.h[0]);
            break;
        case 1:
            v = D_800A1A00.b[1] + D_800A1A00.h[1];
            if (v >= 1000) {
                v = 999;
            }
            func_80060590(v);
            func_8005E8A4(4, 2);
            func_8005FDF0(D_800A1A00.b[1]);
            func_8005E8A4(5, 0);
            func_8005FF28(D_800A1A00.h[1]);
            break;
        case 2:
            v = D_800A1A00.b[2] + D_800A1A00.h[2];
            if (v >= 1000) {
                v = 999;
            }
            func_80060590(v);
            func_8005E8A4(4, 2);
            func_8005FDF0(D_800A1A00.b[2]);
            func_8005E8A4(5, 0);
            func_8005FF28(D_800A1A00.h[2]);
            break;
        }
        func_8005E8A4(-45, -10);
        func_8005EB64(135);
        func_8005E8A4(25, 0);
        func_8005EB64(136);
    } else {
        func_8005E8A4(92, 0);
        func_8006055C(D_8009CFD8);
        func_8005E8A4(-138, 31);
        func_8005EB64(D_8009CFD0 + 140);
        func_8005E8A4(66, -1);
        func_8005B91C(D_8009CFD0, D_8009CFDC, &buf[0], 0);
        func_800605F8(buf[0] + 1);
        func_8005BA78(D_8009CFD0, D_8009CFDC, &buf[1], &buf[2]);
        func_8005E8A4(2, 0);
        func_8006062C(buf[1], buf[2]);
    }
}
