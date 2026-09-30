typedef struct {
    unsigned int f0;
    unsigned char pad4[0xA];
    unsigned char fE;
    unsigned char padF[0xE5];
    unsigned char fF4;
} Sys;

typedef struct {
    unsigned char pad0[0x10];
    int f10;
} Task;

extern unsigned int D_800B0CD8;
extern unsigned int D_8009D1A0;
extern Task *D_8009D300;
extern int D_8009CE00;

extern void func_80042EDC(void);
extern void func_80042F20(void);
extern int func_8006D60C(int a0);
extern int func_8006914C(int a0);
extern void func_80029810(int a0);

int func_800144FC(unsigned char **a0) {
    Sys *s = (Sys *)&D_800B0CD8;

top:
    {
        switch (s->fF4) {
        case 0:
            if (!(s->fE & 3)) {
                s->fF4 = 0x37;
            }
            s->f0 |= 0x800000;
            goto yield;
        case 0x37:
            if (!(D_800B0CD8 & 0x400000)) {
                func_80042EDC();
            }
            s->fF4 = 0x38;
            goto top;
        case 0x38:
            if (func_8006D60C(1) == 1) {
                goto yield;
            }
            if (!(D_800B0CD8 & 0x400000)) {
                func_80042F20();
            }
            s->fF4 = 0x39;
            goto top;
        case 0x39:
            if (func_8006914C(1) == 1) {
                goto yield;
            }
            s->fF4 = 0x3A;
            goto top;
        case 0x3A:
            D_8009D1A0 |= 2;
            func_80029810(**a0);
            s->fF4 = 0x3B;
            goto yield;
        case 0x3B:
            if (s->fE & 3) {
                goto yield;
            }
            s->fF4 = 0;
            s->f0 &= ~0x800000;
            break;
        }
    }
    return 1;
yield:
    D_8009CE00 -= 0xC;
    D_8009D300->f10 = 1;
    return 0;
}
