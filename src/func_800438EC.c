/*
 * func_800438EC — VRAM 0x800438EC / file 0x340EC / size 0xEC (59 words).
 * Boot/menu setup path: allocate two nodes (func_80062D2C / func_8006322C),
 * install the func_80043DA4 and func_8004F838 callbacks, run func_80062CB8,
 * derive a 9-bit flag count from D_8009CEF0 (masked by 0x1F or 0x1EF
 * depending on func_8005B89C) and hand it to func_800647D0, then clear
 * D_8009CEFC / set D_8009CEF8 and run the 800439D8 / 8004C594 tail.
 * The gp globals live at gp+0x180/0x188/0x18C. Build: era -O2 -G8.
 */

typedef struct Obj Obj;
struct Obj {
    char pad00[0x2C];
    void (*f2C)(void);            /* +0x2C */
    void (*f30)(void);            /* +0x30 */
};

extern int D_8009CEF0;
extern int D_8009CEF8;
extern int D_8009CEFC;
extern void func_80043DA4(void);
extern void func_8004F838(void);
extern int func_80062A34(int a, int b);
extern Obj *func_80062D2C(int a, int b, int c, int d);
extern Obj *func_8006322C(int a, Obj *b, Obj *c);
extern void func_80062CB8(Obj *o);
extern int func_8005B89C(void);
extern void func_800647D0(Obj *o, int n);
extern void func_800439D8(void);
extern void func_8004C594(void);

void func_800438EC(void) {
    Obj *a;
    Obj *b;
    int flags;
    int sum;
    int i;

    if (func_80062A34(1, 0) != 0) {
        return;
    }
    a = func_80062D2C(0, 0, 0, 0);
    b = func_8006322C(0, a, a);
    a->f2C = func_80043DA4;
    b->f30 = func_8004F838;
    func_80062CB8(b);
    if (func_8005B89C() != 0) {
        flags = D_8009CEF0 & 0x1F;
    } else {
        flags = D_8009CEF0 & 0x1EF;
    }
    sum = 0;
    for (i = 8; i >= 0; i--) {
        sum += flags & 1;
        flags >>= 1;
    }
    func_800647D0(b, sum);
    D_8009CEFC = 0;
    D_8009CEF8 = 1;
    func_800439D8();
    func_8004C594();
}
