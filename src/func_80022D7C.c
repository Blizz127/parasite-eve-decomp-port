typedef struct Inner {
    unsigned char pad0[0x10];
    unsigned int f10;
} Inner;

typedef struct Rec {
    unsigned char pad0[0x68];
    Inner *f68;
} Rec;

typedef struct Actor {
    unsigned char *f0;
    unsigned char pad4[0x264];
    short f268;
    short f26A;
    short f26C;
} Actor;

extern Rec *D_8009D278;
extern int D_8009D208;
extern int func_8006F39C(int, Actor *);
extern void func_8006F6D4(int, int, int, int, int, int);
extern void func_8006DE80(int, int, int, int, int);

void func_80022D7C(Actor *a)
{
    unsigned int *st = (unsigned int *)(a->f0 + 0xCC);

    if ((D_8009D278->f68->f10 & 0x400) && (*st & 3) != 1) {
        D_8009D208 = func_8006F39C(7, a);
        func_8006F6D4(D_8009D208, 0, 0, 2, 0, 0);
        func_8006DE80(0x484, 0, a->f268, a->f26A, a->f26C);
    } else if ((D_8009D278->f68->f10 & 0x300) == 0x300 && (*st & 0x3C000) != 0x14000) {
        D_8009D208 = func_8006F39C(0x5A, a);
        func_8006DE80(0x488, 0, a->f268, a->f26A, a->f26C);
    } else if ((D_8009D278->f68->f10 & 0x100) && (*st & 0xC000) != 0x4000) {
        D_8009D208 = func_8006F39C(0x58, a);
        func_8006DE80(0x488, 0, a->f268, a->f26A, a->f26C);
    } else if ((D_8009D278->f68->f10 & 0x200) && (*st & 0x30000) != 0x10000) {
        D_8009D208 = func_8006F39C(0x59, a);
        func_8006DE80(0x486, 0, a->f268, a->f26A, a->f26C);
    } else if ((D_8009D278->f68->f10 & 0x800) && (*st & 0xC) != 4) {
        D_8009D208 = func_8006F39C(7, a);
        func_8006F6D4(D_8009D208, 0, 0, 1, 0, 0);
        func_8006DE80(0x482, 0, a->f268, a->f26A, a->f26C);
    } else if ((D_8009D278->f68->f10 & 0x1000) && (*st & 0x30) != 0x10) {
        D_8009D208 = func_8006F39C(7, a);
        func_8006F6D4(D_8009D208, 0, 0, 0, 0, 0);
        func_8006DE80(0x480, 0, a->f268, a->f26A, a->f26C);
    }
}
