typedef struct {
    short x;
    short y;
    short w;
    short h;
} RECT;

extern unsigned short D_800F336C;
extern unsigned short D_800E1204[];
extern unsigned short D_800E21A8[];
extern void func_800750CC();
extern void func_8007506C();

void func_800CF4B4(int a0, int a1, unsigned short *a2)
{
    RECT r;
    unsigned short i;
    unsigned short *buf;

    r.x = (a0 & 0xF) << 4;
    r.y = D_800E1204[D_800F336C] + a0 / 16;
    r.w = 16;
    r.h = 1;
    if (a1 == -1) {
        func_800750CC(&r, a2);
        return;
    }
    buf = D_800E21A8;
    buf[0] = a2[0];
    for (i = 1; i < 16; i++) {
        buf[a1 % 15 + 1] = a2[i];
        a1++;
    }
    func_8007506C(&r, buf);
}
