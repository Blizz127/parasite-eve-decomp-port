typedef struct {
    int f0;
    short f4;
    short f6;
} Slot;

typedef struct {
    unsigned char pad0[0x10];
    unsigned int f10;
    unsigned char f14;
    unsigned char f15;
    unsigned char f16;
    unsigned char f17;
} Info;

typedef struct {
    unsigned char pad0[0x68];
    Info *f68;
} Rec;

extern Slot D_800BE830[];
extern int D_8009D200;
extern int D_8009D2FC;
extern int D_8009D258;
extern int D_8009D208;
extern Rec *D_8009D278;
extern unsigned char D_8009CE44;
extern unsigned char D_8009CE40;
extern unsigned char D_8009D294;
extern unsigned char D_8009D2D8;
extern unsigned char D_8009D1DC;
extern unsigned char D_8009CE38[4];
extern unsigned int D_8009D1AC;
extern unsigned char D_8009D1CE;
extern unsigned char D_8009CE60;

extern void func_80070064(void);
extern void func_800374E8(void);
extern void func_80026FD0(void);

void func_80020F18(void) {
    unsigned char i;

    func_80070064();
    D_8009D200 = -1;
    D_8009D2FC = -1;
    D_8009D258 = -1;
    D_8009D208 = -1;
    for (i = 0; i < 45; i++) {
        D_800BE830[i].f0 = 0;
        (&D_800BE830[i])->f6 = 0;
        D_800BE830[i].f4 = 0;
    }
    D_8009CE44 = 0;
    D_8009CE40 = 0;
    D_8009D294 = 0;
    D_8009D2D8 = (D_8009D278->f68->f10 >> 4) & 3;
    D_8009D1DC = D_8009D278->f68->f10 & 0xF;
    D_8009CE38[0] = D_8009D278->f68->f14;
    D_8009CE38[1] = D_8009D278->f68->f15;
    D_8009CE38[2] = D_8009D278->f68->f16;
    D_8009CE38[3] = D_8009D278->f68->f17;
    func_800374E8();
    D_8009D1CE = 0;
    D_8009D1AC &= ~0x300;
    func_80026FD0();
    D_8009CE60 = 0;
}
