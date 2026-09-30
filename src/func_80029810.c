typedef struct {
    short a;
    short b;
} Pair;

typedef struct {
    unsigned char pad0[0x18];
    int f18;
} Stats;

typedef struct {
    void *f0;
    unsigned char pad4[0x190];
    void *f194;
    unsigned char pad198[0xA0];
    Stats *f238;
} Actor;

typedef struct {
    unsigned char pad0[0x8];
    int f8;
    unsigned char padC[0x6];
    unsigned char f12;
    unsigned char pad13[0x15];
    int f28;
    unsigned char pad2C[0x8];
    int f34;
} Rec;

extern Actor *D_8009D254;
extern int D_8009D1E8;
extern int D_8009D290;
extern int D_8009D28C;
extern unsigned char D_8009CE7C;
extern unsigned char D_8009CE78;
extern unsigned char D_8009D288;
extern unsigned char D_8009CE74;
extern Rec *D_8009D278;
extern unsigned int D_8009D1AC;
extern int D_8009D1A8;
extern unsigned char D_8009D1CE;
extern unsigned char D_8009D235;
extern int D_8009D304;
extern short D_8009D21C;
extern Pair D_800A7FF0[];
extern int D_800B8A90[];
extern int D_8009D250;
extern short D_8009D27C;

extern void func_80020EFC(void);
extern void func_80071A64(int a0);
extern void func_800293F4(int a0);
extern void func_800209F0(void);
extern void func_80030640(void);
extern void func_8001D268();
extern void func_800339A0(int a0);
extern void func_8001A680(Actor *a0, int a1);

void func_80029810(unsigned char id) {
    unsigned char i;
    Rec *r;

    D_8009D1E8 = 0;
    D_8009D290 = 0;
    D_8009D28C = 0;
    D_8009CE7C = 0;
    D_8009CE78 = 0;
    D_8009D288 = 0;
    D_8009CE74 = 0;
    D_8009D278 = (Rec *)D_8009D254->f0;
    func_80020EFC();
    *(unsigned char *)&D_8009D1AC = 0;
    D_8009D1A8 = 0;
    D_8009D1CE = 0;
    D_8009D235 = 0;
    D_8009D304 = 0;
    D_8009D21C = 0;
    {
        register unsigned int fl asm("$3") = D_8009D1AC;

        D_8009D1AC = fl & ~0x300;
    }
    for (i = 0; i < 10; i++) {
        D_800A7FF0[i].a = 0;
        D_800A7FF0[i].b = -1;
    }
    for (i = 0; i < 7; i++) {
        D_800B8A90[i] = 0;
    }
    func_80071A64(D_8009D250);
    func_800293F4(0);
    func_800209F0();
    r = D_8009D278;
    if (r->f8 <= 0) {
        r->f8 = 0x10000;
    } else if (r->f8 >= r->f28) {
        r->f8 = r->f28;
        r->f34 = 0xF0;
    }
    func_80030640();
    D_8009D254->f194 = func_8001D268;
    D_8009D27C = D_8009D254->f238->f18 - 100;
    func_800339A0(id);
    func_8001A680(D_8009D254, D_8009D278->f12);
}
