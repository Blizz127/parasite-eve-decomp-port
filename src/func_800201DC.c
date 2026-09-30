typedef struct {
    unsigned char pad0[0xC];
    unsigned short fC;
    unsigned short fE;
    unsigned char pad10[0x28];
    unsigned short f38;
    unsigned char f3A;
    unsigned char f3B;
    unsigned char pad3C[0x24];
    unsigned short f60;
    unsigned short f62;
    unsigned short f64;
    unsigned char f66;
    unsigned char f67;
} Rec;

extern Rec *D_8009D278;
extern unsigned char *D_8009D254;

void func_800201DC(void) {
    Rec *p = D_8009D278;

    if (p->f3A >= p->f3B) {
        p->fC -= p->f38;
        p->fE -= p->f38;
        p->f3A = 0;
        p->f60 = p->f38;
        D_8009D278->f62 = *(unsigned short *)(D_8009D254 + 0x210);
        D_8009D278->f64 = *(unsigned short *)(D_8009D254 + 0x212);
        D_8009D278->f66 = 30;
        D_8009D278->f67 = 3;
    }
    D_8009D278->f3A++;
}
