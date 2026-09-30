extern int D_8009CF30;

extern unsigned char *func_80062A20(unsigned char *, int);
extern unsigned char *func_80062A34(int, int);
extern void func_80063158(unsigned char *, int, int);
extern void func_80063198(unsigned char *);

void func_80048F24(void) {
    register unsigned char *p asm("$16");
    unsigned char *q;
    register int v asm("$2");
    register int off asm("$17");
    register int t asm("$5");

    p = func_80062A34(1, 0xF);
    q = func_80062A20(func_80062A34(1, 0xD), 0);
    v = D_8009CF30;
    t = *(int *)(p + 0x18);
    if (v == 0) {
        v = *(int *)(q + 0x80);
        if (v == 0) {
            v = 0x9C;
        } else {
            v = 0xA2;
        }
    } else {
        v = 0xB0;
    }
    off = v - t;
    func_80063158(p, off, 0);
    func_80063198(p);
    p = func_80062A34(1, 0xB);
    func_80063158(p, off, 0);
    func_80063198(p);
    p = func_80062A34(1, 0x2F);
    func_80063158(p, off, 0);
    func_80063198(p);
}
