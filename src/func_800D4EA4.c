typedef struct { short vx, vy, vz, pad; } SVEC;

extern int D_800E27EC;
extern unsigned char *D_800E2368;
extern void *D_8009D254;
extern short D_800F3374;
extern unsigned char D_800E1540[];
extern unsigned char D_800E1518[];
extern void func_800CE870();
extern void func_800CF3AC();
extern int func_80077CF4();
extern void func_800D004C();

int func_800D4EA4(int a0) {
    SVEC v30;
    SVEC v38;
    unsigned char *p;
    int t;
    int w;

    switch (a0) {
    case 0:
        break;
    case 1:
        if (D_800E27EC >= 0x14) {
            return 1;
        }
        break;
    case 2:
        D_800F3374 = 0xC8;
        func_800CE870(D_8009D254, 1, &v30);
        v30.vy = v30.vy - 0x12C;
        p = &D_800E1540;
        if (*(short *)(D_800E2368 + 0x1E) == 1) {
            p = &D_800E1518;
        }
        func_800CF3AC(p, &v38, D_800E27EC * 24 / 20);
        t = func_80077CF4((D_800E27EC << 10) / 20);
        w = t / 2 + 0x800;
        func_800D004C(&v30, 0x190, 0x190, 8, 0, w, w, &v38, 0, 0x80, 1);
        break;
    }
    return 0;
}
