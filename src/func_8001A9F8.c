extern unsigned int D_8009D2E8;
extern void *D_8009D254;
extern unsigned int *D_8009D20C;
extern void func_8001D170(void *a0);
extern void func_8001AE40(void *a0);

void func_8001A9F8(void) {
    unsigned int *p;

    if (D_8009D2E8 & 4) {
        func_8001D170(D_8009D254);
    }
    p = D_8009D20C;
    while (p != 0) {
        if ((p[0x26] & 0x80) == 0) {
            func_8001AE40(p);
        }
        p = (unsigned int *)p[1];
    }
}
