extern unsigned char *D_800E2248;
extern unsigned char *D_800F34F4;
extern unsigned char *D_800F3330;
extern int D_800F33B0;
extern unsigned char D_800C20C8[];
extern void func_80071A74(unsigned char *a0);

int func_800C2414(unsigned char *a0, void **a1) {
    register short i asm("$17");
    register void **q asm("$18");
    int f78;
    register short t asm("$2");

    q = a1;
    i = 0;
    f78 = *(int *)(a0 + 0x78);
    D_800E2248 = a0 + 0xC;
    D_800F34F4 = a0 + 0x80;
    D_800F3330 = a0 + 0x200;
    D_800F33B0 = f78;
    do {
        unsigned char *e = (unsigned char *)(i * 6 + (unsigned int)D_800F34F4);
        if (*(signed char *)(e + 1) == 1) {
            void (*f)(unsigned char *, unsigned char *, unsigned char *);
            f = (void (*)(unsigned char *, unsigned char *, unsigned char *))q[*e];
            if (f != (void (*)(unsigned char *, unsigned char *, unsigned char *))-1) {
                f(a0, e, D_800F3330 + *(short *)(e + 4));
            } else {
                func_80071A74(D_800C20C8);
            }
        }
        t = i + 1;
        i = t;
    } while (t < 0x40);
    return 0;
}
