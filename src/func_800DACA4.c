typedef struct {
    unsigned char b0;
    unsigned char b1;
    unsigned char b2;
    unsigned char b3;
} B4;

extern B4 D_800C22E8;
extern B4 D_800C22EC;
extern int D_800E27EC;
extern unsigned char *D_800F33E0;
extern void *D_8009D254;
extern short D_800F3374;
extern unsigned char D_800DAB98[];
extern int func_80071A54();
extern int func_80077CF4(int a0);
extern int func_800CE560();
extern short *func_800CE610();
extern void func_800CE870();
extern void func_800CF5B0();
extern void func_800D004C();
extern void func_800D0728();

int func_800DACA4(int a0, short *a1)
{
    short pos[4];
    B4 blk;
    B4 blk2;
    short *p;
    void *q;
    int t;
    int T;
    int s0v;
    int s1v;

    blk = D_800C22E8;
    blk2 = D_800C22EC;
    switch (a0) {
    case 0:
        t = func_80071A54();
        q = D_8009D254;
        *(int *)(a1 + 4) = t;
        func_800CE870(q, 0, a1);
        return func_800CE560(*(void **)(D_800F33E0 + 8), 8, 0x18, D_800DAB98);
    case 1:
        if (D_800E27EC < 0x33) {
            p = func_800CE610(*(void **)(D_800F33E0 + 8));
            if (p != 0) {
                p[1] = (func_80071A54() & 0xFF) + 500;
                p[3] = (func_80071A54() & 3) + 0x16;
                p[2] = *(int *)(a1 + 4);
                *(int *)(a1 + 4) += (func_80071A54() & 0x1F) + 0x8AA;
            }
        }
        if (D_800E27EC < 0x50) {
            break;
        }
        return 1;
    case 2:
        T = D_800E27EC;
        if (T < 0x51) {
            D_800F3374 = 0x3C;
            pos[0] = a1[0];
            pos[1] = a1[1];
            pos[2] = a1[2];
            s1v = (T << 12) / 80;
            s0v = func_80077CF4((T << 11) / 80) / 32;
            func_800D004C(pos, 800, 800, 0x10, 0, s1v, s1v, &blk, 0, s0v, 1);
            func_800D004C(pos, 250, 250, 8, 0, s1v, s1v, &blk2, 0, s0v, 1);
            func_800D0728(pos, 590, 670, 0x10, 0, s1v, s1v, 0, &blk, s0v, 3);
        }
        D_800F3374 = 0x40;
        func_800CF5B0(a1, 0);
        break;
    default:
        return 0;
    }
    return 0;
}
