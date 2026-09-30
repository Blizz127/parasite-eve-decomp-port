typedef struct {
    unsigned char pad0[0x6];
    short f6;
    unsigned char pad8[0x4];
    unsigned int fC;
    unsigned int f10;
} Obj;

typedef struct {
    unsigned char pad0[0x68];
    Obj *f68;
} Rec;

extern Rec *D_8009D278;
extern void *D_8009D254;
extern int D_8009D200;
extern int D_8009D2FC;

extern void func_800518A8(void);
extern int func_8006F39C(int a0, void *a1);

void func_80021128(void) {
    register Obj *o asm("$4") = D_8009D278->f68;

    if (o->f6 == 6) {
        if (!(o->fC & 0x3FF)) {
            func_800518A8();
        }
        D_8009D200 = func_8006F39C(3, D_8009D254);
        return;
    }
    if (o->f6 == 8) {
        D_8009D2FC = func_8006F39C(6, D_8009D254);
        return;
    }
    if (!(o->fC & 0x3FF)) {
        func_800518A8();
    }
    {
    Obj *q = D_8009D278->f68;
    if (q->f6 == 5 || (q->f10 & 0x1F00)) {
        D_8009D200 = func_8006F39C(0, D_8009D254);
        D_8009D2FC = func_8006F39C(5, D_8009D254);
    } else {
        D_8009D200 = (q->f6 == 2 || (q->f10 & 0xC0) == 0x80) ? func_8006F39C(1, D_8009D254)
                                                              : func_8006F39C(2, D_8009D254);
        D_8009D2FC = func_8006F39C(4, D_8009D254);
    }
    }
}
