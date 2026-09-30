typedef struct {
    int f0;
    short f4;
    short f6;
} Slot;

typedef struct {
    unsigned char pad0[0xE];
    unsigned char fE;
    unsigned char padF[0x59];
    int f68;
    int f6C;
    int f70;
    unsigned char pad74[0x24];
    unsigned int f98;
} Actor;

typedef struct {
    unsigned char pad0[0x12];
    unsigned char f12;
} Rec;

extern unsigned char D_8009D1D4;
extern unsigned char D_8009CE3C;
extern Slot D_800BE830[];
extern Actor *D_8009D254;
extern unsigned int D_8009D2E8;
extern Rec *D_8009D278;
extern int D_8009D28C;

extern void func_80021F38(void);
extern void func_80022210(void);
extern void func_80022394(void);
extern void func_8001A680(Actor *a0, int a1);
extern signed char func_800255E4(void);

void func_80021DE0(void) {
    if (D_8009D1D4 < D_8009CE3C) {
        Slot *s = &D_800BE830[D_8009D1D4];
        Actor *a = D_8009D254;
        short v;

        a->f68 = 0;
        a->f6C = 0;
        a->f70 = 0;
        if (a->fE < 4) {
            return;
        }
        v = s->f4;
        if (v < 3) {
            func_80021F38();
        } else if (v < 0x183) {
            func_80022210();
        } else if (v < 0x197) {
            func_80022394();
        } else if (v < 0x199) {
            func_8001A680(a, 0xD);
            D_8009D2E8 |= 1;
            D_8009D1D4++;
            D_8009D254->f98 |= 0x100;
        } else {
            if (func_800255E4() == 1) {
                func_8001A680(D_8009D254, D_8009D278->f12);
                D_8009D28C = 4;
            }
            D_8009D1D4++;
        }
    } else {
        D_8009D1D4 = 0;
        D_8009CE3C = 0;
    }
}
