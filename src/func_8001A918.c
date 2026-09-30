typedef struct {
    unsigned char pad0[0x2];
    unsigned short count;
    unsigned char pad4[0x4];
    unsigned short f8;
    unsigned char padA[0xE];
    unsigned int f18;
    unsigned int f1C;
    unsigned int f20;
    unsigned int f24;
    unsigned int f28[1];
} Hdr;

extern Hdr *D_800B1620;
extern Hdr *D_8009D1FC;
extern unsigned int *D_8009CE08;
extern unsigned int D_8009D1D8;
extern unsigned int D_8009CE14;

void func_8001A918(void) {
    register Hdr *h asm("$4");
    register unsigned int f asm("$3");
    Hdr *g;
    unsigned int *p;
    unsigned int i;

    {
        register Hdr *tmp asm("$2") = D_800B1620;

        h = tmp;
    }
    f = h->f18;
    D_8009D1FC = h;
    if (f > 0x80000000) {
        {
            register unsigned int *q asm("$2") = h->f28;
            D_8009CE08 = q;
        }
        D_8009D1D8 = h->f20;
        return;
    }
    h->f18 = (unsigned int)h + f;
    h->f1C = (unsigned int)h + h->f1C;
    h->f24 = (unsigned int)h + h->f24;
    if (h->f20 != 0) {
        h->f20 = (unsigned int)h + h->f20;
    }
    g = D_8009D1FC;
    p = g->f28;
    D_8009CE08 = p;
    D_8009D1D8 = g->f20;
    D_8009CE14 = g->f24;
    g->f8 = (g->f8 >> 5) + 1;
    for (i = 0; i < g->count; i++) {
        register unsigned int t asm("$4") = g->f28[i] + (unsigned int)g;

        *p++ = t;
    }
}
