typedef struct {
    unsigned char pad0[0x8];
    unsigned short flags;
    unsigned char padA[0x6];
    int f10;
} Task;

extern unsigned int D_800B0CD8[];
extern unsigned int D_8009D1A0;
extern Task *D_8009D300;
extern int D_8009CE00;

extern void func_80067CBC(void);
extern void func_8004E704(int a0);

int func_80016E40(int **a0) {


    if (!(D_800B0CD8[0] & 0x1000)) {
        Task *t = D_8009D300;

        if (!(t->flags & 0x20)) {
            t->flags |= 0x20;
            t->f10 = 1;
            D_8009CE00 -= 0xC;
            return 0;
        }
        func_80067CBC();
        func_8004E704(*a0[0]);
        D_800B0CD8[0] |= 0x9000;
        D_8009D1A0 |= 4;
        D_8009D300->flags &= ~0x20;
    }
    return 1;
}
