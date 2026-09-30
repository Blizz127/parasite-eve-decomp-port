typedef struct Inner {
    unsigned char pad0[2];
    short f2;
} Inner;

typedef struct Rec {
    unsigned char pad0[0x4C];
    unsigned int f4C;
    unsigned char pad50[0x18];
    Inner *f68;
} Rec;

typedef struct Sub {
    unsigned char pad0[7];
    unsigned char f7;
} Sub;

typedef struct Actor {
    Sub *f0;
    unsigned char pad4[0x1B0];
    unsigned char f1B4[4];
} Actor;

typedef struct Ent {
    Actor *a;
    int hp;
    int f8;
} Ent;

extern Rec *D_8009D278;
extern signed char D_8009D2B0;
extern unsigned int D_8009D1F4;
extern Ent D_8009E000[];
extern int D_800B0E08[];
extern signed char D_8009CE44;
extern void func_800275CC(Ent *, int);
extern void func_80026FD0(void);
extern void func_8006DF50(int, int, int, int, int);
extern void func_800347B4(void);
extern void func_800314E4(unsigned char *, int, int, int);
extern void func_80026FF8(Ent *, int, signed char);

void func_800258CC(signed char mode)
{
    signed char f;

    if (D_8009D2B0 >= 2) {
        if ((unsigned char)(mode - 1) >= 2) {
            if (D_8009D1F4 & 0x10) {
                func_800275CC(D_8009E000, D_8009CE44);
                D_8009CE44 = (D_8009CE44 + 1) % D_8009D2B0;
                func_80026FD0();
                if (D_800B0E08[0] != 0) {
                    func_8006DF50(D_800B0E08[0], 0x44E, 0, 0x80, 0x7F);
                }
            }
            if (D_8009D1F4 & 0x40) {
                func_800275CC(D_8009E000, D_8009CE44);
                D_8009CE44 = (D_8009CE44 + D_8009D2B0 - 1) % D_8009D2B0;
                func_80026FD0();
                if (D_800B0E08[0] != 0) {
                    func_8006DF50(D_800B0E08[0], 0x44E, 0, 0x80, 0x7F);
                }
            }
        }
    } else if (D_8009D2B0 == 0) {
        mode = 8;
    }
    switch (mode) {
    case 0:
    case 1:
    case 2:
    case 3:
        func_800347B4();
        if ((D_8009D278->f4C & 0x30) == 0x10) {
            f = D_8009E000[D_8009CE44].hp > 200;
        } else {
            f = D_8009E000[D_8009CE44].hp > D_8009D278->f68->f2;
        }
        func_800314E4(D_8009E000[D_8009CE44].a->f1B4, mode, f, D_8009E000[D_8009CE44].a->f0->f7);
        break;
    case 4:
    case 5:
    case 6:
    case 7:
        func_800314E4(D_8009E000[D_8009CE44].a->f1B4, mode, 0, D_8009E000[D_8009CE44].a->f0->f7);
        break;
    case 8:
        func_800314E4(0, 8, 0, 0);
        break;
    }
    func_80026FF8(D_8009E000, D_8009CE44, mode);
}
