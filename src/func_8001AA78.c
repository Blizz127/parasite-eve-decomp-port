typedef struct {
    unsigned char f0;
    unsigned char f1;
    unsigned char pad2[0x14];
} EntA; /* 22 bytes */

typedef struct {
    unsigned char pad0[0x2];
    unsigned short f2;
    int f4;
    unsigned char pad8[0x14];
} EntB; /* 28 bytes */

typedef struct {
    int f0;
    int f4;
    int f8;
} Coef;

typedef struct {
    unsigned char pad0[0x2];
    unsigned short f2;
    unsigned char pad4[0x18];
    void *f1C;
} Hdr;

typedef struct {
    unsigned char pad0[0x28];
    int f28;
    int f2C;
    int f30;
    unsigned char pad34[0x64];
    unsigned int f98;
    unsigned char pad9C[0x108];
    void *f1A4;
    void *f1A8;
} Obj;

#define HI(p, off) (*(unsigned short *)((unsigned char *)(p) + (off)))

extern Coef *D_8009D1D8;
extern Hdr *D_8009D1FC;
extern unsigned short **D_8009CE08;

extern int func_8003708C(int a0, int a1);
extern int func_8001C614(void *a0, int a1, int a2);

void func_8001AA78(Obj *o) {
    unsigned short x;
    unsigned short z;
    unsigned int i;
    unsigned int j;
    unsigned int n;
    unsigned short *g;
    unsigned short *ix;

    if (o->f98 & 0x80) {
        return;
    }
    x = HI(o, 0x2A);
    z = HI(o, 0x32);
    for (i = 0; i < D_8009D1FC->f2; i++) {
        if (D_8009D1D8 == 0) {
            int h;

            {
                register unsigned short **tbl asm("$2") = D_8009CE08;

                g = tbl[i];
            }
            h = ((short *)g)[0];
            n = g[1];
            ix = g + 2;
            for (j = 0; j < n; j++, ix++) {
                EntA *e = (EntA *)D_8009D1FC->f1C + *ix;

                if (func_8001C614(e, (short)x, (short)z)) {
                    o->f1A4 = e;
                    o->f1A8 = e;
                    if (!(o->f98 & 2)) {
                        o->f2C = h << 16;
                    }
                    return;
                }
            }
        } else {
            {
                register unsigned short **tbl asm("$2") = D_8009CE08;

                g = tbl[i];
            }
            n = g[2];
            ix = g + 3;
            for (j = 0; j < n; j++, ix++) {
                EntB *e = (EntB *)D_8009D1FC->f1C + *ix;

                if (func_8001C614(e, (short)x, (short)z)) {
                    int a;
                    int b;

                    o->f1A4 = e;
                    o->f1A8 = e;
                    if (!(o->f98 & 2)) {
                        a = func_8003708C(D_8009D1D8[e->f2].f0, o->f28);
                        b = func_8003708C(D_8009D1D8[e->f2].f8, o->f30);
                        o->f2C = func_8003708C(e->f4 - a - b, D_8009D1D8[e->f2].f4);
                    }
                    return;
                }
            }
        }
    }
}
