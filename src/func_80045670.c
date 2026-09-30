typedef struct {
    unsigned char pad0[7];
    unsigned char f7;
    unsigned char f8;
    unsigned char f9;
    unsigned char padA[4];
    short fE;
    short f10;
    short f12;
} Item;

extern int D_8009CF1C;
extern int D_8009CF18;
extern void func_8005E8A4(int, int);
extern void func_8005EB64(int);
extern void func_8005FF28(int);
extern void func_8005E988(int, int);
extern void func_80060590(int);
extern void func_8005E968(int);

static inline int clamp999(int x)
{
    if (x >= 1000) {
        x = 999;
    }
    return x;
}

void func_80045670(Item *a, Item *b)
{
    int c;

    if (b == 0) {
        return;
    }
    if (a != 0) {
        if (D_8009CF1C != 0) {
            func_8005E8A4(0x28, -0x32);
            func_8005EB64(0x88);
            func_8005E8A4(0, 10);
            func_8005FF28(a->fE);
            func_8005E8A4(-0x14, 0xE);
            func_8005FF28(a->f10);
            func_8005E8A4(-0x14, 0xE);
            func_8005FF28(a->f12);
            func_8005E8A4(-0x26, 0xC);
        }
        func_8005E8A4(0x26, -0xC);
        func_8005EB64(0x22);
        func_8005E8A4(0, -0xE);
        func_8005EB64(0x22);
        func_8005E8A4(0, -0xE);
        func_8005EB64(0x22);
        if (D_8009CF1C != 0) {
            func_8005E8A4(0xC, -0xA);
            func_8005EB64(0x88);
            func_8005E988(a->fE, b->fE);
            func_8005E8A4(0, 10);
            func_8005FF28(b->fE);
            func_8005E8A4(-0x14, 0xE);
            func_8005E988(a->f10, b->f10);
            func_8005FF28(b->f10);
            func_8005E8A4(-0x14, 0xE);
            func_8005E988(a->f12, b->f12);
            func_8005FF28(b->f12);
        } else {
            func_8005E988(clamp999(a->f7 + a->fE), clamp999(b->f7 + b->fE));
            func_8005E8A4(0xC, -2);
            func_80060590(clamp999(b->f7 + b->fE));
            func_8005E8A4(-0x24, 0xE);
            func_8005E988(clamp999(a->f8 + a->f10), clamp999(b->f8 + b->f10));
            func_80060590(clamp999(b->f8 + b->f10));
            func_8005E8A4(-0x24, 0xE);
            func_8005E988(clamp999(a->f9 + a->f12), clamp999(b->f9 + b->f12));
            func_80060590(clamp999(b->f9 + b->f12));
        }
    } else {
        c = 0x7F;
        if (D_8009CF18 != 0) {
            c = 0x7C;
        }
        func_8005E8A4(4, 0x17);
        func_8005EB64(c);
        func_8005E8A4(0, 0xE);
        func_8005EB64(c + 1);
        func_8005E8A4(0, 0xE);
        func_8005EB64(c + 2);
        func_8005E968(0x8080);
        func_8005E8A4(0x28, -0x1C);
        func_8005E988(0, b->f7 + b->fE);
        func_80060590(clamp999(b->f7 + b->fE));
        func_8005E8A4(-0x24, 0xE);
        func_8005E988(0, b->f8 + b->f10);
        func_80060590(clamp999(b->f8 + b->f10));
        func_8005E8A4(-0x24, 0xE);
        func_8005E988(0, b->f9 + b->f12);
        func_80060590(clamp999(b->f9 + b->f12));
    }
    func_8005E968(0x808080);
}
