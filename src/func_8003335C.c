typedef struct { unsigned char f0; unsigned char f1; unsigned short f2; unsigned char pad[24]; } Src;
typedef struct { unsigned char pad0[8]; short f8; short fA; unsigned char fC; unsigned char fD; short fE; unsigned char pad1[12]; } Dst;

extern Src D_8009E974[2][13];
extern Dst D_8009EC40[];
extern unsigned char D_8009D235;

void func_8003335C(unsigned char *a0, int a1, int a2) {
    unsigned char i;
    unsigned char c;
    Dst *base;
    Dst *p;

    D_8009D235 = 0x1E;
    i = 0;
    base = D_8009EC40;
    c = a2;
    do {
        p = (Dst *)((int)(i * 28) + (int)base);
        p->fC = D_8009E974[i][c].f0;
        p->fD = D_8009E974[i][c].f1;
        p->f8 = *(unsigned short *)(a0 + 0x210) - 8;
        p->fA = *(unsigned short *)(a0 + 0x212) - 0x10;
        D_8009EC40[i].fE = D_8009E974[i][c].f2;
        i++;
    } while (i < 2);
}
