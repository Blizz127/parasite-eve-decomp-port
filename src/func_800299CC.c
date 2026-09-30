typedef struct {
    unsigned char pad0[0x10];
    unsigned int f10;
} Mdl;

typedef struct {
    unsigned char pad0[0x10];
    unsigned short f10;
    unsigned char f12;
    unsigned char pad13[0x39];
    unsigned int f4C;
    unsigned char f50[6];
    unsigned char f56;
    unsigned char pad57[9];
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
    unsigned char pad1C[0xE];
    short f2A;
    unsigned char pad2C[2];
    short f2E;
    unsigned char pad30[2];
    short f32;
    unsigned char pad34[0x64];
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

#define G4 D_800B00E8[D_8009CDDC]
#define P28 D_800B6928[D_8009CDDC]

void func_800299CC(void)
{
    char pad[0x1A0];
    int s;
    unsigned char k;
    Ent *e;

    s = 1;
    if (D_8009D278->f4C & 0x80000) {
        if (D_8009D28C == 6) {
            D_8009CE7C = 6;
            D_8009D28C = 0;
        }
    }
    if (!(D_8009D278->f4C & 0x80000)) {
        if (D_8009CE7C == 6) {
            D_8009CE7C = 0;
            D_8009D28C = 6;
        }
    }
    D_8009D230 = 0;
    D_8009D278 = D_8009D254->f0;
    D_8009D2A4 = func_8005C498(D_800A76D8);
    if (D_8009D28C == 0 && D_8009D244 != 0) {
        if (D_8009D278->f10 >= 9000) {
            D_8009D278->f10 = 9000;
            if (D_8009D288 == 0) {
                func_80071A64(D_8009D250);
                if (D_800B0E08[0] != 0) {
                    func_8006DF50(D_800B0E08[0], 0x454, 0, 0x80, 0x7F);
                }
                D_8009D288 = 1;
            }
            if (!(D_8009D278->f4C & 0x2000) && D_8009D2A0 != 0) {
                if ((D_8009D250 & 3) == 0) {
                    G4.r0 = 0; G4.g0 = 0x46; G4.b0 = 0x82;
                    G4.r1 = 0x9F; G4.g1 = 0xFF; G4.b1 = 0xF9;
                    G4.r2 = 0; G4.g2 = 0x46; G4.b2 = 0x82;
                    G4.r3 = 0x9F; G4.g3 = 0xFF; G4.b3 = 0xF9;
                    P28.r0 = 0x9F; P28.g0 = 0xFF; P28.b0 = 0xF9;
                } else if ((D_8009D250 & 3) == 1) {
                    register int ka asm("$5") = 0x50;
                    register int kb asm("$6") = 0xA3;
                    register int kc asm("$4") = 0xBE;
                    G4.r0 = ka; G4.g0 = kb; G4.b0 = kc;
                    G4.r1 = ka; G4.g1 = kb; G4.b1 = kc;
                    G4.r2 = ka; G4.g2 = kb; G4.b2 = kc;
                    G4.r3 = ka; G4.g3 = kb; G4.b3 = kc;
                    P28.r0 = ka; P28.g0 = kb; P28.b0 = kc;
                } else if ((D_8009D250 & 3) == 2) {
                    register int ma asm("$7") = 0x9F;
                    register int mb asm("$8") = 0xFF;
                    register int mc asm("$6") = 0xF9;
                    register int md asm("$5") = 0x46;
                    register int me asm("$4") = 0x82;
                    G4.r0 = ma; G4.g0 = mb; G4.b0 = mc;
                    G4.r1 = 0; G4.g1 = md; G4.b1 = me;
                    G4.r2 = ma; G4.g2 = mb; G4.b2 = mc;
                    G4.r3 = 0; G4.g3 = md; G4.b3 = me;
                    P28.r0 = 0; P28.g0 = md; P28.b0 = me;
                } else if ((D_8009D250 & 3) == 3) {
                    register int na asm("$5") = 0x50;
                    register int nb asm("$6") = 0xA3;
                    register int nc asm("$4") = 0xBE;
                    G4.r0 = na; G4.g0 = nb; G4.b0 = nc;
                    G4.r1 = na; G4.g1 = nb; G4.b1 = nc;
                    G4.r2 = na; G4.g2 = nb; G4.b2 = nc;
                    G4.r3 = na; G4.g3 = nb; G4.b3 = nc;
                    P28.r0 = na; P28.g0 = nb; D_800B6928[D_8009CDDC_b].b0 = nc;
                }
                if (func_80021054() == 0) {
                    k = 0;
                    if (D_8009D1F4 & 0x200) {
                        D_8009D1F0 = 0;
                        k = 1;
                    } else if (D_8009D1F4 & 0x80) {
                        k = 1;
                        D_8009D1F0 = 1;
                        func_8005C174(1);
                        func_80067CBC();
                    }
                    if (k) {
                        func_800866A4(0, 0xFF);
                        D_8009D28C = 1;
                        D_8009D290 = 0;
                        D_8009D1A0 |= 4;
                        s = 0;
                        D_8009D2B0 = func_8002156C();
                        func_80020F18();
                        if (D_8009D2B0 != 0) {
                            func_80043240(0);
                        } else {
                            func_80043240(1);
                        }
                    }
                }
            }
        }
        if (func_80021054() > 0) {
            if (!(D_8009D278->f4C & 0x4000) || (D_8009D1A0 & 0x100)) {
                func_80021DE0();
                s = 2;
            }
            if (D_8009D294 != 0) {
                func_800236E8();
            }
        }
        if (D_8009D1AC & 0x300) {
            func_8003495C();
        }
        func_8001D340(s);
        for (e = D_8009D20C; e != 0; e = e->next) {
            if (e != D_8009D254 && e->f0 != 0) {
                func_80027D14(e);
            }
        }
        if (D_8009D23C != 0) {
            func_8006F6D4(D_8009D2FC, 0, 1, (int)D_800B8A90, 0, 0);
            func_8006F6D4(D_8009D2FC, 0, 0, 1, 0, 0);
            D_8009D23C = 0;
        }
        if (D_8009D235 != 0) {
            func_80033430();
        }
        if (D_8009D254->fF == D_8009D254->f18.h.hi && D_8009D254->fE < 4) {
            if (D_8009D298[0] == 0) {
                func_8001A680(D_8009D254, D_8009D278->f12);
                if (!(D_800B0CE6 & 1) && func_80021054() == 0 && D_8009D28C == 0
                    && (D_8009D278->f68->f10 & 0x8000) && D_8009D1D0 != 0) {
                    func_800306E0(D_8009D1D0);
                    D_8009D1D0 = 0;
                }
            } else {
                if (D_8009D298[0] >= 2) {
                    D_8009D254->f98 |= 0x100;
                }
                D_8009D298[0] = 0;
                func_8001A680(D_8009D254, D_8009D29A[0]);
                D_8009D254->f14 = D_8009D29C[0];
                D_8009D254->f18.w = D_8009D29C[0] - 0x10000;
            }
        }
        if (D_8009D254->fE == 0xD && !(D_800B0CE6 & 1)) {
            D_8009D278->f12 = 4;
            if (D_8009D254->f98 & 0x100) {
                func_8006DE80(0x453, 0, D_8009D254->f2A, D_8009D254->f2E, D_8009D254->f32);
                D_8009D254->f98 &= ~0x100;
            }
            if (D_8009D254->fF == D_8009D254->f18.h.hi) {
                func_8001A680(D_8009D254, D_8009D278->f12);
                D_8009D2E8 &= ~1;
            }
        }
        D_8009D1E8++;
    } else if (D_8009D28C == 1) {
        D_8009D28C = func_80025EE8();
        if (D_8009D28C == 0) {
            D_8009D1A0 &= ~4;
        }
    } else if (D_8009D28C == 2) {
        func_8002B0E8();
    } else if (D_8009D28C == 3) {
        if (D_8009D278->f4C & 0x800) {
            if (func_8002AA98()) {
                D_8009D278->f4C &= ~0x800;
            }
        } else if (func_80053E6C(0x12)) {
            if (func_8002AA98()) {
                func_8005409C(0x12);
            }
        } else {
            func_8002B29C();
        }
        if (D_8009D278->f56 != 0) {
            func_80032B0C(0, D_8009D278->f50);
            D_8009D278->f56--;
        }
        if (D_8009D278->f66 != 0) {
            func_80032B0C(0, D_8009D278->f60);
            D_8009D278->f66--;
        }
        for (e = D_8009D20C; e != 0; e = e->next) {
            if (e != D_8009D254 && e->f0 != 0) {
                func_80027D14(e);
            }
        }
    } else if (D_8009D28C == 4) {
        func_8002B94C();
    } else if (D_8009D28C == 5) {
        func_8002F0B0();
    } else if (D_8009D28C == 6) {
        func_8002BC90();
    } else if (D_8009D28C == 7) {
        func_8002D1F0();
    } else if (D_8009D28C == 8) {
        func_8002DC58();
    }
    if (D_8009D1CE != 0 && D_8009D28C == 0) {
        func_80034DE0();
    }
    if (D_8009D244 != 0) {
        func_80033A40();
    }
    if (D_8009D2A4 != 0) {
        func_80067CBC();
    }
}
