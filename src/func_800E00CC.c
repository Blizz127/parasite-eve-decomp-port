typedef struct {
    unsigned char f0;
    unsigned char f1;
    unsigned char f2;
    unsigned char f3;
    unsigned short f4;
    unsigned short f6;
    unsigned short f8;
    unsigned char fA;
    unsigned char fB;
    short fC;
    short fE;
    unsigned char f10;
    unsigned char f11;
    unsigned char f12;
    unsigned char f13;
} Rec;

extern short D_800E21A4;
extern Rec *D_800E2800;

void func_800E00CC(unsigned short *a0, unsigned char a1, short a2, unsigned char a3,
                   unsigned char a4, unsigned char a5, unsigned char a6)
{
    Rec *p;
    short i;

    p = D_800E2800;
    for (i = 0; p->f0 != 0; i++) {
        if (i > D_800E21A4) {
            break;
        }
        p--;
    }
    if (i > D_800E21A4) {
        p = D_800E2800;
    }
    p->fA = a1;
    p->f4 = a0[0];
    p->f6 = a0[1];
    p->f8 = a0[2];
    p->f1 = a3;
    p->f2 = a3;
    p->f3 = 0;
    p->f0 = 1;
    p->f10 = a4;
    p->f11 = a5;
    p->f12 = a6;
    p->fE = a2;
    p->fC = 0;
    if (D_800E21A4 < 0x14) {
        D_800E21A4++;
    }
}
