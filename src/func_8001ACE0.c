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
    unsigned char pad0[0x1C];
    void *f1C;
} Hdr;

typedef struct {
    unsigned char pad0[0x28];
    int f28;
    int f2C;
    int f30;
    unsigned char pad34[0xC];
    int f40;
    int f44;
    int f48;
    unsigned char pad4C[0x158];
    void *f1A4;
    void *f1A8;
} Obj;

/* f2A / f32 are the high halves of f28 / f30 */
#define HI(p, off) (*(short *)((unsigned char *)(p) + (off)))

extern Coef *D_8009D1D8;
extern Hdr *D_8009D1FC;
extern short **D_8009CE08;

extern int func_8003708C(int a0, int a1);
extern void func_8001C614(void *a0, int a1, int a2);

void func_8001ACE0(Obj *o, unsigned short idx) {
    if (D_8009D1D8 == 0) {
        EntA *e = (EntA *)D_8009D1FC->f1C + idx;

        o->f1A4 = e;
        o->f1A8 = e;
        o->f2C = *D_8009CE08[e->f1] << 16;
        func_8001C614(e, HI(o, 0x2A), HI(o, 0x32));
    } else {
        EntB *e = (EntB *)D_8009D1FC->f1C + idx;
        int a;
        int b;

        o->f1A4 = e;
        o->f1A8 = e;
        a = func_8003708C(D_8009D1D8[e->f2].f0, o->f28);
        b = func_8003708C(D_8009D1D8[e->f2].f8, o->f30);
        o->f2C = func_8003708C(e->f4 - a - b, D_8009D1D8[e->f2].f4);
        func_8001C614(e, HI(o, 0x2A), HI(o, 0x32));
    }
    o->f40 = o->f28;
    o->f44 = o->f2C;
    o->f48 = o->f30;
}
