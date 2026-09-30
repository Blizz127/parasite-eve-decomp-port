typedef struct {
    int f0;
    short f4;
    short f6;
} Slot;

typedef struct {
    unsigned char pad0[0xE];
    unsigned char fE;
    unsigned char fF;
    unsigned char pad10[0xA];
    unsigned short f1A;
    unsigned char pad1C[0xE];
    short x;
    short pad2C;
    short y;
    short pad30;
    short z;
} Actor;

typedef struct {
    unsigned char pad0[0x12];
    unsigned char f12;
    unsigned char pad13[0x39];
    unsigned int f4C;
} Rec;

extern Rec *D_8009D278;
extern Actor *D_8009D254;
extern unsigned int D_8009D1A0;
extern unsigned char D_8009D1D4;
extern Slot D_800BE830[];

extern void func_8001A680(Actor *a0, int a1);
extern void func_8006DE80(int id, int a1, int x, int y, int z);
extern void func_80023E14(int a0);
extern int func_8006F39C(int a0, void *a1);

void func_80022210(void) {
    if (D_8009D278->f4C & 0x200000) {
        func_8001A680(D_8009D254, 0xE);
        func_8006DE80(0x4B3, 0, D_8009D254->x, D_8009D254->y, D_8009D254->z);
        D_8009D1A0 |= 0x100;
        D_8009D278->f4C &= ~0x200000;
    }
    if (D_8009D254->fF == D_8009D254->f1A) {
        func_8001A680(D_8009D254, D_8009D278->f12);
        D_8009D278->f4C |= 0x200000;
        func_80023E14(D_800BE830[D_8009D1D4].f4 - 3);
        func_8006F39C(0x55, D_8009D254);
        D_8009D1D4++;
        if (D_800BE830[D_8009D1D4].f4 < 3 || D_800BE830[D_8009D1D4].f4 >= 0x197 || D_8009D254->fE < 4) {
            D_8009D1A0 &= ~0x100;
        }
    }
}
