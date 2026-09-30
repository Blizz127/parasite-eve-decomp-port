extern unsigned int D_800BCF88;
extern unsigned int D_800BCF8C;
extern unsigned int D_800BCF98;
extern short D_800BCF9C;
extern short D_800BCF9E;
extern short D_800BCFA0;
extern short D_800BCFA2;

int func_800661EC(short a0, short a1, short a2, int a3)
{
    unsigned int *f = &D_800BCF88;
    unsigned int s;
    unsigned int m;

    s = *f;
    if ((s & 0x40) == 0) {
        return -19;
    }
    D_800BCFA0 = 1;
    D_800BCF9C = a0;
    D_800BCF9E = a1;
    D_800BCFA2 = a2;
    D_800BCF98 = D_800BCF8C;
    m = s & ~0xF;
    if (a3 == 8) {
        *f = m | 9;
    } else {
        *f = m | 1;
    }
    return 0;
}
