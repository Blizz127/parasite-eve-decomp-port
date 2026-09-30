typedef struct { short vx, vy, vz, pad; } SVEC;

extern int D_800E27EC;
extern void *D_8009D254;
extern short D_800F3374;
extern unsigned char D_800E1494[];
extern void func_800CE870();
extern void func_800CF3AC();
extern int func_80077CF4();
extern void func_800D0728();

int func_800DF87C(int a0) {
    SVEC v30;
    SVEC v38;
    SVEC v40;
    int t;

    switch (a0) {
    case 0:
        break;
    case 1:
        if (D_800E27EC >= 0x18) {
            return 1;
        }
        break;
    case 2:
        D_800F3374 = 0;
        func_800CE870(D_8009D254, 1, &v30);
        v30.vy = v30.vy - 0x12C;
        func_800CF3AC(&D_800E1494, &v40, D_800E27EC);
        v38.vy = 0;
        v38.vx = 0x400;
        v38.pad = 1;
        v38.vz = D_800E27EC << 8;
        t = func_80077CF4((D_800E27EC << 10) / 24);
        func_800D0728(&v30, 0x12C, 0x190, 0xA, &v38, t, t, 0, &v40, 0x80, 1);
        break;
    }
    return 0;
}
