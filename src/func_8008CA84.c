extern int D_8009D2F4;
extern unsigned char D_800B8628[];
extern void (*D_8009C0C0[])(unsigned char *a0);

void func_8008CA84(void) {
    unsigned char *p;

    p = D_800B8628;
    if (D_8009D2F4 != 0) {
        do {
            D_8009C0C0[*p](p);
            D_8009D2F4 = D_8009D2F4 - 1;
            p += 0x24;
        } while (D_8009D2F4 != 0);
    }
}
