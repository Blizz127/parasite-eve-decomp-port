typedef struct { short x, y, w, h; } RECT;

extern unsigned char D_800BCE91;
extern unsigned char D_800BCEA5;
extern short D_800BCE9E;
extern short D_800BCE8A;
extern short D_800BCE9C;
extern short D_800BCE88;
extern short D_800BCEA2;
extern short D_800BCE8E;
extern unsigned char D_800BCDC8;
extern unsigned char D_800BCE3C;
extern unsigned char D_800BCDE0;
extern unsigned char D_800BCE3A;
extern unsigned char D_800BCDDE;
extern unsigned char D_800BCE3B;
extern unsigned char D_800BCDDF;
extern short D_800BCE38;
extern short D_800BCDDC;
extern unsigned char D_800BCE3D;
extern unsigned char D_800BCDE1;
extern unsigned char D_800BCE3E;
extern unsigned char D_800BCDE2;
extern unsigned char D_800BCE3F;
extern unsigned char D_800BCDE3;
extern int D_8009CDDC;

extern void func_80074A44();
extern void func_80074BB8();
extern void func_80074D28();
extern void func_80074F44();
extern void func_800749D8();
extern void func_80074924();
extern void func_800755F0();

void func_8003E754(int a0, int a1) {
    register int one asm("$17");
    RECT r;
    unsigned char *p;
    unsigned char *q;
    unsigned char *base;

    func_80074A44(0);
    func_80074BB8(0);
    func_80074D28(0);
    r.w = 0x3FF;
    r.h = 0x200;
    r.x = 0;
    r.y = 0;
    func_80074F44(&r, 0, 0, 1);
    p = &D_800BCE91;
    q = p - 0x11;
    one = 1;
    *p = one;
    D_800BCEA5 = one;
    func_800749D8(q, 0, a1, a0, a1);
    func_800749D8(p + 3, 0, 0, a0, a1);
    D_800BCE9E = 8;
    D_800BCE8A = 8;
    {
        short t = a1;
        D_800BCE9C = 0;
        D_800BCE88 = 0;
        D_800BCEA2 = t;
        D_800BCE8E = t;
    }
    base = &D_800BCDC8;
    func_80074924(base, 0, 0, a0, a1);
    func_80074924(base + 0x5C, 0, a1, a0, a1);
    D_800BCE3C = one;
    D_800BCDE0 = one;
    D_800BCE3A = one;
    D_800BCDDE = one;
    D_800BCE3B = 0;
    D_800BCDDF = 0;
    D_800BCE38 = 0;
    D_800BCDDC = 0;
    D_800BCE3D = 0;
    D_800BCDE1 = 0;
    D_800BCE3E = 0;
    D_800BCDE2 = 0;
    D_800BCE3F = 0;
    D_800BCDE3 = 0;
    D_8009CDDC = 0;
    func_800755F0(q);
}
