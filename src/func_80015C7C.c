typedef struct {
    unsigned char pad0[0x8];
    unsigned short flags;
    unsigned char padA[0x6];
    int f10;
} Task;

extern Task *D_8009D300;
extern int D_8009CE00;
extern unsigned int D_800B0CD8[];
extern unsigned int D_8009D1A0;
extern short D_8009D2A4;

extern int func_8005D2B4(int a0, int a1, int a2, int *a3);
extern int func_800629B0(void);
extern void func_80067CBC(void);

int func_80015C7C(int **a0) {
    if (!(D_8009D300->flags & 0x20)) {
        *a0[3] = func_8005D2B4(*a0[0], *a0[1], *a0[2], a0[4]);
    }
    if (func_800629B0()) {
        Task *t = D_8009D300;

        if (!(t->flags & 0x20)) {
            t->flags |= 0x20;
            D_800B0CD8[0] |= 0x9000;
            func_80067CBC();
        } else {
            D_8009D1A0 |= 4;
        }
        D_8009CE00 -= 0x1C;
        D_8009D300->f10 = 1;
        return 0;
    }
    if (D_8009D2A4 != 0) {
        *a0[3] = D_8009D2A4;
        D_8009D300->flags &= ~0x20;
    }
    return 1;
}
