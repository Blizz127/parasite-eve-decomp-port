typedef struct {
    unsigned char pad0[0x8];
    unsigned short flags;
    unsigned char padA[0x6];
    int f10;
} Task;

typedef struct {
    unsigned char pad0[0x98];
    unsigned int f98;
    unsigned char pad9C[0x118];
    unsigned char f1B4[0x9C];
    unsigned short f250;
} State;

extern Task *D_8009D300;
extern State *D_8009D2F0;
extern State *D_8009D254;
extern unsigned int D_8009D2E8;
extern unsigned char D_800B0CEC[];
extern unsigned short D_800B0D88;
extern int D_8009CE00;

extern void func_8003C5D8(void *a0, int a1);

int func_80014FD8(short **a0) {
    Task *t = D_8009D300;

    if (t->flags & 0x20) {
        if (!(D_8009D2F0->f250 & 4)) {
            t->flags &= ~0x20;
            return 1;
        }
    } else {
        D_8009D2F0->f98 &= ~0x40;
        t->flags |= 0x20;
        func_8003C5D8(D_8009D2F0->f1B4, **a0);
        D_8009D2F0->f250 |= 4;
        if (D_8009D2F0 == D_8009D254) {
            D_8009D2E8 &= ~2;
            func_8003C5D8(D_800B0CEC, **a0);
            D_800B0D88 |= 4;
        }
    }
    D_8009CE00 -= 0xC;
    D_8009D300->f10 = 1;
    return 0;
}
