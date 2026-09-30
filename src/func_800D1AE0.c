typedef struct {
    unsigned int tag;
    unsigned char b4;
    unsigned char b5;
    unsigned char b6;
    unsigned char b7;
    short w8;
    short wA;
    short wC;
    short wE;
} Prim;

extern int D_8009CDD8;
extern int D_8009CDDC;
extern int D_800B0E58[];
extern void func_80077C44();
extern int func_80077A64();
extern void func_80077C84();

void func_800D1AE0(unsigned char *a0, int a1, int a2, int a3)
{
    Prim *p;
    Prim *q;
    unsigned int *ot;
    int base;
    int t;

    p = (Prim *)(D_800B0E58[D_8009CDDC] + D_8009CDD8);
    D_8009CDD8 = D_8009CDD8 + 0x10;
    func_80077C44(p);
    p->b4 = a0[0] * a1 / 128;
    p->b5 = a0[1] * a1 / 128;
    p->b6 = a0[2] * a1 / 128;
    if ((unsigned int)a3 < 0x1000) {
        p->wC = 0x140;
        p->w8 = 0;
        p->wA = 0;
        p->wE = 0xF0;
        ot = (unsigned int *)(a3 * 4 + D_800B0E58[D_8009CDDC - 8]);
        if (a2 != 0xFF) {
            base = D_8009CDD8;
            D_8009CDD8 = base + 8;
            q = (Prim *)(D_800B0E58[D_8009CDDC] + base);
            t = func_80077A64(0, a2, 0, 0);
            func_80077C84(q, 0, 1, t & 0xFFFF);
            if (p != 0) {
                p->b7 |= 2;
                p->tag = (p->tag & 0xFF000000) | (*ot & 0xFFFFFF);
                *ot = (*ot & 0xFF000000) | ((unsigned int)p & 0xFFFFFF);
            }
            q->tag = (q->tag & 0xFF000000) | (*ot & 0xFFFFFF);
            *ot = (*ot & 0xFF000000) | ((unsigned int)q & 0xFFFFFF);
        } else {
            if (p != 0) {
                p->tag = (p->tag & 0xFF000000) | (*ot & 0xFFFFFF);
                *ot = (*ot & 0xFF000000) | ((unsigned int)p & 0xFFFFFF);
            }
        }
    }
}
