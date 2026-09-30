/* VRAM 0x800471E4 / file 0x379E4 / size 0x148. */
extern int D_8009CFB8;

extern unsigned char *func_80062A20(int, int);
extern int func_80063428(unsigned char *);
extern unsigned char *func_80062D2C(int, unsigned char *, int, int);
extern unsigned char *func_8006322C(int, unsigned char *, unsigned char *);
extern void func_80063158(unsigned char *, int, int);
extern void func_80062CB8(unsigned char *);
extern void func_800473E4(unsigned char *);
extern void func_8005B71C(int);
extern void func_80046EAC(int);
extern void func_800525EC();
extern void func_80052634();
extern void func_80047354();
extern void func_8004732C();

int func_800471E4(int a0, int pad) {
    unsigned char *p;
    unsigned char *q;
    unsigned char *r;
    register int sel asm("$5");

    if (pad & 0x10000) {
        p = func_80062A20(a0, 0);
        sel = func_80063428(p);
        switch (D_8009CFB8) {
        case 0:
            if (sel == 2) {
                r = func_80062D2C(0x3B, p, 0, 0);
                q = func_8006322C(0x3B, r, r);
                *(unsigned int *)(r + 0x2C) = (unsigned int)func_80047354;
                func_80063158(r, 0x70 - *(int *)(r + 0x18), 0x48 - *(int *)(r + 0x1C));
                *(unsigned int *)(q + 0x30) = (unsigned int)func_8004732C;
                func_80062CB8(q);
                break;
            }
        case 2:
            func_800473E4(p);
            break;
        case 1:
            func_8005B71C(sel);
            func_80046EAC(1);
            break;
        }
        func_800525EC();
        return 1;
    }
    if (pad & 0x40) {
        func_80046EAC(0);
        func_80052634();
    }
    return 1;
}
