extern unsigned short D_800C0E28[];
extern int D_800A18D8[];
extern int D_800A18B4[];
extern int D_800A18FC[];
extern int D_800C0E10;
extern int D_8009CF68;
extern unsigned char D_80092258[];
extern unsigned char D_80092298[];

extern unsigned char *func_80062D2C(int, int, int, int);
extern void func_8005B91C(int, int, int *, int);
extern void func_80043B0C();
extern void func_8004905C();
extern void func_80043C64();

void func_800439D8(void) {
    register unsigned char *p asm("$3");
    unsigned char *q;
    register unsigned char *r asm("$3");
    unsigned short *src;
    int i;
    int *d0;
    int *d3;
    int *d2;

    p = func_80062D2C(0x12, 0, 0, 0);
    *(unsigned int *)(p + 0x30) = (unsigned int)func_80043B0C;
    *(unsigned int *)(p + 0x4C) = (unsigned int)D_80092258;
    q = func_80062D2C(0x18, 0, 0, 0);
    *(unsigned int *)(q + 0x30) = (unsigned int)func_8004905C;
    r = func_80062D2C(0x2D, 0, 0, 0);
    src = D_800C0E28;
    i = 0;
    d0 = D_800A18D8;
    d3 = D_800A18B4;
    d2 = D_800A18FC;
    *(unsigned int *)(r + 0x30) = (unsigned int)func_80043C64;
    *(unsigned int *)(r + 0x4C) = (unsigned int)D_80092298;
    do {
        *d0 = *src;
        src++;
        *d2 = 0;
        func_8005B91C(i, *d0, d3, 0);
        d3++;
        i++;
        d0++;
        d2++;
    } while (i < 7);
    D_8009CF68 = D_800C0E10;
}
