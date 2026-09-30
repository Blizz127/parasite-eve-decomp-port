typedef struct {
    unsigned char pad0[0x10];
    unsigned int f10;
} Mdl;

typedef struct {
    unsigned char pad0[0x10];
    unsigned short f10;
    unsigned char f12;
    unsigned char pad13[0x35];
    unsigned char f48;
    unsigned char f49;
    unsigned char pad4A[2];
    unsigned int f4C;
    unsigned char f50[6];
    unsigned char f56;
    unsigned char pad57;
    unsigned char f58[6];
    unsigned char f5E;
    unsigned char pad5F;
    unsigned char f60[6];
    unsigned char f66;
    unsigned char pad67;
    Mdl *f68;
} St;

typedef struct Ent {
    void *f0;
    struct Ent *next;
    unsigned char pad8[6];
    unsigned char fE;
    unsigned char fF;
    unsigned char pad10[4];
    int f14;
    union {
        int w;
        struct {
            unsigned short lo;
            unsigned short hi;
        } h;
    } f18;
    int f1C;
    unsigned char pad20[0xA];
    short f2A;
    unsigned char pad2C[2];
    short f2E;
    unsigned char pad30[2];
    short f32;
    unsigned char pad34[0x34];
    int f68;
    int f6C;
    int f70;
    unsigned char pad74[0x24];
    unsigned int f98;
} Ent;

typedef struct {
    unsigned int tag;
    unsigned char r0, g0, b0, code;
    short x0, y0;
    unsigned char r1, g1, b1, p1;
    short x1, y1;
    unsigned char r2, g2, b2, p2;
    short x2, y2;
    unsigned char r3, g3, b3, p3;
    short x3, y3;
} PolyG4;

typedef struct {
    unsigned int tag;
    unsigned char r0, g0, b0, code;
    unsigned char pad8[0x14];
} Prim28;

typedef struct {
    PolyG4 a;
    PolyG4 b;
} G4Pair;

typedef struct {
    signed char pad0[5];
    signed char f5;
    signed char f6;
} Sub;

extern G4Pair D_800B0130[];
extern St *D_8009D278;
extern Ent *D_8009D254;
extern Ent *D_8009D20C;
extern int D_8009D28C;
extern unsigned char D_8009CE7C;
extern short D_8009D2A4;
extern unsigned char D_8009D244;
extern unsigned char D_8009D288;
extern signed char D_8009D2A0;
extern signed char D_8009D2B0;
extern int D_8009D1E8;
extern unsigned char D_8009D23C;
extern int D_8009D2FC;
extern void *D_8009D1D0;
extern int D_8009D230;
extern unsigned int D_8009D250;
extern int D_800B0E08[];
extern int D_8009CDDC;
extern int D_8009CDDC_b asm("D_8009CDDC");
extern PolyG4 D_800B00E8[];
extern Prim28 D_800B6928[];
extern unsigned int D_8009D1F4;
extern unsigned char D_8009D1F0;
extern unsigned int D_8009D1A0;
extern int D_8009D290;
extern unsigned char D_8009D294;
extern unsigned int D_8009D1AC;
extern unsigned char D_800B8A90[];
extern unsigned char D_8009D235;
extern unsigned short D_8009D298[1];
extern unsigned char D_800B0CE6;
extern unsigned char D_8009D29A[1];
extern int D_8009D29C[1];
extern unsigned int D_8009D2E8;
extern unsigned int D_8009D2E8_s[1] asm("D_8009D2E8");
extern unsigned char D_8009D1CE;
extern unsigned char D_800A76D8[];

extern short func_8005C498(void *);
extern void func_80071A64(unsigned int);
extern void func_8006DF50(int, int, int, int, int);
extern signed char func_80021054(void);
extern void func_8005C174(int);
extern void func_80067CBC(void);
extern void func_800866A4(int, int);
extern signed char func_8002156C(void);
extern void func_80020F18(void);
extern void func_80043240(int);
extern void func_80021DE0(void);
extern void func_800236E8(void);
extern void func_8003495C(void);
extern void func_8001D340(int);
extern void func_80027D14(Ent *);
extern void func_8006F6D4(int, int, int, int, int, int);
extern void func_80033430(void);
extern void func_8001A680(Ent *, int);
extern void func_800306E0(void *);
extern void func_8006DE80(int, int, int, int, int);
extern signed char func_80025EE8(void);
extern void func_8002B0E8(void);
extern unsigned char func_8002AA98(void);
extern int func_80053E6C(int);
extern void func_8005409C(int);
extern void func_8002B29C(void);
extern void func_80032B0C(int, void *);
extern void func_8002B94C(void);
extern void func_8002F0B0(void);
extern void func_8002BC90(void);
extern void func_8002D1F0(void);
extern void func_8002DC58(void);
extern void func_80034DE0(void);
extern void func_80033A40(void);
extern void func_80021D4C(void);
extern void func_800374E8(void);
extern void func_80036254(Ent *);
extern int func_8006914C(int);

#define G4 D_800B00E8[D_8009CDDC]
#define P28 D_800B6928[D_8009CDDC]
#define GB D_800B0130[D_8009CDDC].b
#define RGB(p, n, _r, _g, _b) (p).r##n = (_r), (p).g##n = (_g), (p).b##n = (_b)

void func_8002BC90(void)
{
    char pad[0x300];
    unsigned char s;
    Ent *e;

    func_80021D4C();
    s = 1;
    func_800374E8();
    { register unsigned int t asm("$3") = D_8009D2E8_s[0] | 1; D_8009D2E8_s[0] = t; }
    D_8009D254->f68 = 0;
    D_8009D254->f6C = 0;
    D_8009D254->f70 = 0;
    D_8009D1A0 &= ~4;
    if (D_8009D278->f10 >= 9000) {
        if ((D_8009D250 & 3) == 0) {
            RGB(G4, 0, 0, 0x46, 0x82); RGB(G4, 1, 0x9F, 0xFF, 0xF9);
            RGB(G4, 2, 0, 0x46, 0x82); RGB(G4, 3, 0x9F, 0xFF, 0xF9);
            RGB(P28, 0, 0x9F, 0xFF, 0xF9);
        } else if ((D_8009D250 & 3) == 1) { { register int ka asm("$5") = 0x50; register int kb asm("$6") = 0xA3; register int kc asm("$4") = 0xBE;
            RGB(G4, 0, ka, kb, kc); RGB(G4, 1, ka, kb, kc); 
            RGB(G4, 2, ka, kb, kc); RGB(G4, 3, ka, kb, kc);
            RGB(P28, 0, ka, kb, kc); }
        } else if ((D_8009D250 & 3) == 2) { { register int k9 asm("$7") = 0x9F; register int kf asm("$8") = 0xFF;
            RGB(G4, 0, k9, kf, 0xF9); RGB(G4, 1, 0, 0x46, 0x82); 
            RGB(G4, 2, k9, kf, 0xF9); RGB(G4, 3, 0, 0x46, 0x82);
            RGB(P28, 0, 0, 0x46, 0x82); }
        } else if ((D_8009D250 & 3) == 3) { { register int ka asm("$5") = 0x50; register int kb asm("$6") = 0xA3; register int kc asm("$4") = 0xBE;
            RGB(G4, 0, ka, kb, kc); RGB(G4, 1, ka, kb, kc); 
            RGB(G4, 2, ka, kb, kc); RGB(G4, 3, ka, kb, kc);
            P28.r0 = ka; P28.g0 = kb; D_800B6928[D_8009CDDC_b].b0 = kc; }
        }
    }
    if (D_8009D278->f4C & 0x2000) {
        if ((D_8009D250 & 3) == 0) {
            RGB(GB, 0, 0xFF, 0x3D, 0x81); RGB(GB, 1, 0x83, 0x13, 1);
            RGB(GB, 2, 0xFF, 0x3D, 0x81); RGB(GB, 3, 0x83, 0x13, 1);
        } else if ((D_8009D250 & 3) == 1) {
            RGB(GB, 0, 0xC1, 0x28, 0x41); RGB(GB, 1, 0xC1, 0x28, 0x41);
            RGB(GB, 2, 0xC1, 0x28, 0x41); RGB(GB, 3, 0xC1, 0x28, 0x41);
        } else if ((D_8009D250 & 3) == 2) {
            RGB(GB, 0, 0x83, 0x13, 1); RGB(GB, 1, 0xFF, 0x3D, 0x81);
            RGB(GB, 2, 0x83, 0x13, 1); RGB(GB, 3, 0xFF, 0x3D, 0x81);
        } else if ((D_8009D250 & 3) == 3) {
            RGB(GB, 0, 0xC1, 0x28, 0x41); RGB(GB, 1, 0xC1, 0x28, 0x41);
            RGB(GB, 2, 0xC1, 0x28, 0x41);
            GB.r3 = 0xC1; GB.g3 = 0x28; D_800B0130[D_8009CDDC_b].b.b3 = 0x41;
        }
    }
    if (D_8009D278->f56 != 0) {
        func_80032B0C(0, D_8009D278->f50);
        D_8009D278->f56--;
        s = 0;
    }
    if (D_8009D278->f5E != 0) {
        func_80032B0C(0, D_8009D278->f58);
        D_8009D278->f5E--;
        s = 0;
    }
    if (D_8009D278->f66 != 0) {
        func_80032B0C(0, D_8009D278->f60);
        D_8009D278->f66--;
        s = 0;
    }
    for (e = D_8009D20C; e != 0; e = e->next) {
        if (e == D_8009D254) {
            if (e->fE == D_8009D278->f12) {
                continue;
            }
            s = 0;
            e->f98 &= ~0x100;
            if (e->fF != e->f18.h.hi) {
                continue;
            }
            if (e->fE < 4 && D_8009D298[0] != 0) {
                if (D_8009D298[0] >= 2) {
                    e->f98 |= 0x100;
                }
                D_8009D298[0] = 0;
                func_8001A680(D_8009D254, D_8009D29A[0]);
                D_8009D254->f14 = D_8009D29C[0];
                D_8009D254->f18.w = D_8009D29C[0] - 0x10000;
            } else {
                func_8001A680(e, D_8009D278->f12);
            }
        } else if (e->f0 != 0 && ((Sub *)e->f0)->f5 != 1) {
            if ((unsigned int)(e->fE - 2) >= 2) {
                e->f98 &= ~0x1000;
                func_80036254(e);
                s = 0;
                if (e->fF == e->f18.h.hi) {
                    func_8001A680(e, (unsigned short)((Sub *)e->f0)->f6);
                } else if (e->f1C != 0x10000) {
                    e->f1C = 0x10000;
                }
            }
            func_80027D14(e);
        }
    }
    if (func_8006914C(0)) {
        s = 0;
    }
    if (s) {
        func_800866A4(0, 0xFF);
        { St *p = D_8009D278; p->f49 = 0; p->f48 = 0; }
        D_8009D28C = 7;
        RGB(D_800B00E8[0], 0, 0, 0x46, 0x82); RGB(D_800B00E8[0], 1, 0x9F, 0xFF, 0xF9);
        RGB(D_800B00E8[0], 2, 0, 0x46, 0x82); RGB(D_800B00E8[0], 3, 0x9F, 0xFF, 0xF9);
        RGB(D_800B6928[0], 0, 0x9F, 0xFF, 0xF9);
        RGB(D_800B00E8[1], 0, 0, 0x46, 0x82); RGB(D_800B00E8[1], 1, 0x9F, 0xFF, 0xF9);
        RGB(D_800B00E8[1], 2, 0, 0x46, 0x82); RGB(D_800B00E8[1], 3, 0x9F, 0xFF, 0xF9);
        D_8009D2E8_s[0] &= ~1;
        RGB(D_800B6928[1], 0, 0x9F, 0xFF, 0xF9);
        RGB(D_800B0130[0].a, 0, 0, 0x82, 0x36); RGB(D_800B0130[0].a, 1, 0x4A, 0xFF, 0x3B);
        RGB(D_800B0130[0].a, 2, 0, 0x82, 0x36); RGB(D_800B0130[0].a, 3, 0x4A, 0xFF, 0x3B);
        RGB(D_800B0130[1].a, 0, 0, 0x82, 0x36); RGB(D_800B0130[1].a, 1, 0x4A, 0xFF, 0x3B);
        RGB(D_800B0130[1].a, 2, 0, 0x82, 0x36); RGB(D_800B0130[1].a, 3, 0x4A, 0xFF, 0x3B);
        RGB(D_800B0130[0].b, 0, 0xFF, 0x3D, 0x81); RGB(D_800B0130[0].b, 1, 0x83, 0x13, 1);
        RGB(D_800B0130[0].b, 2, 0xFF, 0x3D, 0x81); RGB(D_800B0130[0].b, 3, 0x83, 0x13, 1);
        RGB(D_800B0130[1].b, 0, 0xFF, 0x3D, 0x81); RGB(D_800B0130[1].b, 1, 0x83, 0x13, 1);
        RGB(D_800B0130[1].b, 2, 0xFF, 0x3D, 0x81); RGB(D_800B0130[1].b, 3, 0x83, 0x13, 1);
    }
}
