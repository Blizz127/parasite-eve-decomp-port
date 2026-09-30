extern unsigned char *D_8009D2F0;
extern unsigned char *D_8009D254;
extern unsigned int D_800BCF88[];
extern void func_8001AA78();

int func_80012C20(int **arg) {
    register int **a0 asm("$5");
    register unsigned char *p asm("$3");

    a0 = arg;
    __asm__ __volatile__("" : "=r"(a0) : "0"(a0));
    switch (*a0[0]) {
    case 0:
        {
        unsigned char *c = D_8009D2F0;
        *(int *)(c + 0x28) = *a0[1];
        *(int *)(c + 0x2C) = *a0[2];
        *(int *)(c + 0x30) = *a0[3];
        func_8001AA78(c);
        {
            unsigned char *q = D_8009D2F0;
            unsigned char *r = D_8009D254;
            register int x asm("$3");
            register int y asm("$4");
            int z;
            x = *(int *)(q + 0x28);
            y = *(int *)(q + 0x2C);
            z = *(int *)(q + 0x30);
            *(int *)(q + 0x40) = x;
            *(int *)(q + 0x44) = y;
            *(int *)(q + 0x48) = z;
            if (q == r) {
                unsigned int *f = D_800BCF88;
                *f = *f | 0x80;
            }
        }
        }
        break;
    case 1:
        p = D_8009D2F0;
        *(int *)(p + 0x40) = *a0[1];
        *(int *)(p + 0x44) = *a0[2];
        *(int *)(p + 0x48) = *a0[3];
        break;
    case 2:
        p = D_8009D2F0;
        *(int *)(p + 0x68) = *a0[1];
        *(int *)(p + 0x6C) = *a0[2];
        *(int *)(p + 0x70) = *a0[3];
        break;
    case 3:
        p = D_8009D2F0;
        *(int *)(p + 0x78) = *a0[1];
        *(int *)(p + 0x7C) = *a0[2];
        *(int *)(p + 0x80) = *a0[3];
        break;
    case 4:
        p = D_8009D2F0;
        *(int *)(p + 0x88) = *a0[1];
        *(int *)(p + 0x8C) = *a0[2];
        *(int *)(p + 0x90) = *a0[3];
        break;
    case 5:
        p = D_8009D2F0;
        *(short *)(p + 0x38) = *a0[1];
        *(short *)(p + 0x3A) = *a0[2];
        *(short *)(p + 0x3C) = *a0[3];
        break;
    case 6:
        p = D_8009D2F0;
        *(int *)(p + 0x58) = *a0[1];
        *(int *)(p + 0x5C) = *a0[2];
        *(int *)(p + 0x60) = *a0[3];
        break;
    }
    return 1;
}
