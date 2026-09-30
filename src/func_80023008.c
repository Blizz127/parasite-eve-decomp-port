typedef struct {
    unsigned char pad0[0x6];
    short f6;
    unsigned char pad8[0x4];
    unsigned int fC;
} Obj;

typedef struct {
    unsigned char pad0[0x4C];
    unsigned int f4C;
    unsigned char pad50[0x18];
    Obj *f68;
} Rec;

typedef struct {
    unsigned char pad0[0x2A];
    short x;
    short pad2C;
    short y;
    short pad30;
    short z;
} Actor;

extern Rec *D_8009D278;
extern Actor *D_8009D254;
extern unsigned char D_8009D294;
extern int D_8009D200;

extern void func_8006DE80(int id, int a1, int x, int y, int z);
extern void func_8006DD38(int a0, int a1, int x, int y, int z);
extern void func_8006F6D4(int a0, int a1, int a2, int a3, int a4, int a5);

void func_80023008(void) {
    Rec *r = D_8009D278;
    Obj *o = r->f68;

    if (o->f6 == 8) {
        D_8009D294 = 1;
        func_8006DE80(0x46C, 0, D_8009D254->x, D_8009D254->y, D_8009D254->z);
    } else if (o->fC & 0x3FF) {
        D_8009D294 = 1;
        if (!(r->f4C & 0x100000)) {
            func_8006F6D4(D_8009D200, 0, 0, 1, 0, 0);
            func_8006DD38(0, 0, D_8009D254->x, D_8009D254->y, D_8009D254->z);
        }
        o = D_8009D278->f68;
        o->fC = (o->fC & ~0x3FF) | (((o->fC & 0x3FF) - 1) & 0x3FF);
    } else {
        D_8009D294 = 0;
        func_8006DE80(0x46E, 0, D_8009D254->x, D_8009D254->y, D_8009D254->z);
    }
}
