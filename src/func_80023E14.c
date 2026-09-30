typedef struct Rec {
    unsigned char pad0[8];
    int f8;
    short fC;
    unsigned char padE[0xE];
    short f1C;
    unsigned char pad1E[0xA];
    int f28;
    int f2C;
    int f30;
    unsigned char pad34[0x18];
    unsigned int b0 : 2;
    unsigned int b2 : 2;
    unsigned int b4 : 2;
    unsigned int b6 : 2;
    unsigned int b8 : 4;
    unsigned int b12 : 1;
    unsigned int b13 : 19;
} Rec;

typedef struct Actor {
    Rec *rec;
    unsigned char pad4[0x26];
    short f2A;
    unsigned char pad2C[2];
    short f2E;
    unsigned char pad30[2];
    short f32;
} Actor;

extern unsigned int D_8009D1A0;
extern Actor *D_8009D254;
extern Rec *D_8009D278;
extern unsigned int D_8009D2E8;
extern void func_8006DE80(int, int, int, int, int);
extern void func_800523F8(int *, int *);
extern void func_8003335C(Actor *, int, int);

void func_80023E14(int item)
{
    signed char eff;

    eff = -1;
    if (!(D_8009D1A0 & 2)) {
        D_8009D278 = D_8009D254->rec;
        func_8006DE80(item < 13 ? 0x4B4 : 0x4B5, 1, D_8009D254->f2A, D_8009D254->f2E, D_8009D254->f32);
    } else {
        func_8006DE80(item < 13 ? 0x4B4 : 0x4B5, 0, D_8009D254->f2A, D_8009D254->f2E, D_8009D254->f32);
    }
    switch (item) {
    case 6:
        D_8009D278->fC += 0x2D;
        break;
    case 7:
        D_8009D278->fC += 0x5A;
        break;
    case 8:
        D_8009D278->fC += 0xB4;
        break;
    case 9:
        D_8009D278->fC += 0x190;
        break;
    case 10:
        D_8009D278->fC = D_8009D278->f1C;
        break;
    case 11:
        D_8009D278->f8 += D_8009D278->f28 / 4;
        break;
    case 12:
        D_8009D278->f8 += D_8009D278->f28 / 2;
        break;
    case 13:
        if (D_8009D278->b0 == 0) {
            D_8009D278->b0 = 3;
        } else if (D_8009D278->b0 != 3) {
            D_8009D278->b0 = 0;
        }
        eff = 7;
        break;
    case 14:
        if (D_8009D278->b4 == 0) {
            D_8009D278->b4 = 3;
        } else if (D_8009D278->b4 != 3) {
            D_8009D278->b4 = 0;
        }
        eff = 5;
        break;
    case 15:
        if (D_8009D278->b2 == 0) {
            D_8009D278->b2 = 3;
        } else if (D_8009D278->b2 != 3) {
            D_8009D278->b2 = 0;
            D_8009D2E8 &= ~0x10;
        }
        eff = 4;
        break;
    case 16:
        if (D_8009D278->b6 == 0) {
            D_8009D278->b6 = 3;
        } else if (D_8009D278->b6 != 3) {
            D_8009D278->b6 = 0;
        }
        eff = 6;
        break;
    case 17:
        func_800523F8(&D_8009D278->f2C, &D_8009D278->f30);
        if (D_8009D278->b0 != 3) {
            D_8009D278->b0 = 0;
        }
        if (D_8009D278->b2 != 3) {
            D_8009D278->b2 = 0;
            D_8009D2E8 &= ~0x10;
        }
        if (D_8009D278->b4 != 3) {
            D_8009D278->b4 = 0;
        }
        if (D_8009D278->b6 != 3) {
            D_8009D278->b6 = 0;
        }
        D_8009D278->b12 = 0;
        break;
    }
    if (D_8009D278->fC > D_8009D278->f1C) {
        D_8009D278->fC = D_8009D278->f1C;
    }
    if (eff != -1 && (D_8009D1A0 & 2)) {
        func_8003335C(D_8009D254, 0, (unsigned char)eff);
    }
}
