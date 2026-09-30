extern unsigned char D_800A1A20[];

extern unsigned char *func_80062CC4();
extern unsigned char *func_80062D2C(int, unsigned char *, int, int);
extern unsigned char *func_8006322C(int, unsigned char *, unsigned char *);
extern void func_80062CB8(unsigned char *);
extern void func_8004D024(int);
extern int func_8005DC4C(int);
extern void func_80052BCC(unsigned char *, int);
extern int func_8005F1A0(unsigned char *);
extern void func_8005DE88(void);

extern void func_8004CFD4();
extern void func_8004D030();
extern void func_8004FFA8();

void func_8004CE28(int a, int b) {
    unsigned char *obj;
    unsigned char *obj2;
    int w;

    obj = func_80062D2C(0x28, func_80062CC4(), 0, 1);
    obj2 = func_8006322C(0x28, obj, obj);
    *(unsigned int *)(obj + 0x30) = (unsigned int)func_8004CFD4;
    *(unsigned int *)(obj + 0x2C) = (unsigned int)func_8004D030;
    *(unsigned int *)(obj2 + 0x30) = (unsigned int)func_8004FFA8;
    func_80062CB8(obj2);
    func_8004D024(0);
    func_80052BCC(D_800A1A20, func_8005DC4C(a));
    func_80052BCC(D_800A1A20 + 0x40, func_8005DC4C(b));
    if ((func_8005F1A0(D_800A1A20) > func_8005F1A0(D_800A1A20 + 0x40)
         ? func_8005F1A0(D_800A1A20) : func_8005F1A0(D_800A1A20 + 0x40)) >= 0x64) {
        register unsigned char *q1 asm("$17");
        register unsigned char *q2 asm("$19");
        register int u asm("$16");
        q1 = D_800A1A20;
        u = func_8005F1A0(q1);
        q2 = q1 + 0x40;
        w = func_8005F1A0(u <= func_8005F1A0(q2) ? q2 : q1);
    } else {
        w = 0x64;
    }
    *(int *)(obj + 0x34) = w + 0x14;
    *(int *)(obj + 0x18) = (0x12C - w) >> 1;
    *(int *)(obj + 0x38) = *(int *)(obj + 0x38) + 0xE;
    *(int *)(obj2 + 0x18) = *(int *)(obj + 0x34) - 0x44;
    *(int *)(obj2 + 0x1C) = *(int *)(obj2 + 0x1C) + 0xE;
    func_8005DE88();
}
