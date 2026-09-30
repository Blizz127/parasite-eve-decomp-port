typedef struct { char b[4]; } Blk4;
typedef struct { short vx, vy, vz, pad; } SVEC;

extern int D_800E27EC;
extern unsigned short D_800F336C;
extern int D_800F3428;
extern unsigned short D_800E1204[];
extern unsigned char D_800E18C0[];
extern unsigned char D_800C22DC[];
extern void func_800CF3AC();
extern int func_80077AA4();
extern void func_800CEE20();

int func_800D7A1C(int a0, unsigned short *a1) {
    Blk4 v28;
    SVEC v30;
    unsigned int m;
    unsigned int idx;
    int v;

    v28 = *(Blk4 *)D_800C22DC;
    switch (a0) {
    case 1:
        {
            unsigned short t = *(volatile unsigned short *)(a1 + 3) - 1;
            a1[1] = a1[1] - *(volatile unsigned short *)(a1 + 3);
            a1[3] = t;
        }
        if (D_800E27EC >= 0x18) {
            return 1;
        }
        break;
    case 2:
        func_800CF3AC(&D_800E18C0, &v28, D_800E27EC);
        v30.vx = a1[0];
        v30.vy = a1[1];
        v30.vz = a1[2];
        m = D_800F336C;
        idx = D_800E1204[m];
        if (m == 4) {
            if (D_800F3428 != 0) {
                idx = idx + 4;
            }
        }
        v = func_80077AA4(0x20, idx);
        func_800CEE20(&v30, 0, 0x1000, 0x1000, 0x8B, (unsigned short)v, 1, 0x80, &v28);
        break;
    }
    return 0;
}
