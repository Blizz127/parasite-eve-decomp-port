extern int D_8009CF1C;
extern int D_8009CF18;
extern int D_8009CF0C;

extern int func_80059F08(int);
extern int func_8005415C(int);
extern void func_800543CC(int, int);
extern unsigned char *func_80062A34(int, int);
extern void func_80063158(unsigned char *, int, int);
extern void func_800542A0(int);
extern int func_80054288();
extern void func_800526C4();
extern unsigned char *func_80062D2C(int, int, int, int);
extern unsigned char *func_8006322C(int, unsigned char *, unsigned char *);
extern void func_80064C20(unsigned char *);
extern void func_80064B74(unsigned char *, int);
extern void func_80062CB8(unsigned char *);
extern void func_800647D0(unsigned char *, int);
extern void func_800525EC();
extern void func_800466C0();
extern void func_8004FB48();

void func_80046378(int a0, int a1) {
    int h;
    int mask;
    unsigned char *p;
    unsigned char *q;
    int flag;

    if (D_8009CF1C != 0) {
        h = func_80059F08(0);
        if (func_8005415C(h) != 0 && func_8005415C(h) < 6) {
            mask = 0x3E;
        } else {
            mask = 1 << func_8005415C(h);
        }
        func_800543CC(mask, h);
        p = func_80062A34(1, 0x2F);
        func_80063158(p, 0xB4 - *(int *)(p + 0x18), 0xA4 - *(int *)(p + 0x1C));
    } else {
        func_800542A0(D_8009CF18 != 0 ? 0x1FE : 0x200);
    }
    if (D_8009CF0C != 0 || func_80054288() != 0) {
        p = func_80062D2C(7, a0, 0, 0);
        q = func_8006322C(7, p, p);
        *(unsigned int *)(p + 0x2C) = (unsigned int)func_800466C0;
        flag = D_8009CF1C;
        *(unsigned int *)(q + 0x30) = (unsigned int)func_8004FB48;
        if (flag != 0) {
            func_80064C20(q);
        }
        if (D_8009CF18 == 0) {
            func_80064B74(q, 0x13);
        }
        if (a1 == 0) {
            *(int *)(q + 0x44) = -1;
        } else {
            *(int *)(q + 0x44) = 0;
        }
        if (*(int *)(q + 0x48) < 0) {
            *(int *)(q + 0x48) = 0;
        }
        if (*(int *)(q + 0x44) >= 0) {
            func_80062CB8(q);
        }
        func_800647D0(q, func_80054288());
        if (D_8009CF0C != 0) {
            *(int *)(q + 0x44) = -1;
            func_80063158(p, 0, 0x14);
        }
        func_800525EC();
    } else {
        func_800526C4();
    }
}
