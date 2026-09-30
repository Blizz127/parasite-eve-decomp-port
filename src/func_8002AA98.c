typedef struct Ent {
    unsigned char pad0[0xE];
    unsigned char fE;
    unsigned char fF;
    unsigned char pad10[4];
    int f14;
    int f18_pad;
    unsigned char pad1C[0x2A - 0x1C];
    short f2A;
    unsigned char pad2C[2];
    short f2E;
    unsigned char pad30[2];
    short f32;
    unsigned char pad34[0x98 - 0x34];
    int f98;
    unsigned char pad9C[0x1B4 - 0x9C];
    unsigned char sub[0x210 - 0x1B4];
    unsigned short f210;
    unsigned short f212;
    unsigned char pad214[0x250 - 0x214];
    unsigned short f250;
    unsigned char f252;
} Ent;

typedef struct St {
    unsigned char pad0[8];
    int f8;
    short fC;
    unsigned char padE[0x12 - 0xE];
    unsigned char f12;
    unsigned char pad13[0x1C - 0x13];
    unsigned short f1C;
    unsigned char pad1E[0x2C - 0x1E];
    int f2C;
    int f30;
    unsigned char pad34[0x58 - 0x34];
    short f58;
    short f5A;
    short f5C;
    unsigned char f5E;
    unsigned char f5F;
} St;

typedef struct PolyG4 {
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

typedef struct Sprt {
    unsigned int tag;
    unsigned char r0, g0, b0, code;
    short x0, y0;
    unsigned char u0, v0;
    unsigned short clut;
    short w, h;
} Sprt;

typedef struct SEnt {
    unsigned int tp[2];
    Sprt sp;
} SEnt;

extern Ent *D_8009D254;
extern Ent D_800B0B38;
extern St *D_8009D278;
extern unsigned char D_8009CE70;
extern unsigned char D_8009CE74;
extern signed char D_8009D2A0;
extern int D_8009D28C;
extern PolyG4 D_800B00E8[2];
extern PolyG4 D_800B0130[2][2];
extern SEnt D_800B6920[2];

extern void func_8001A680(Ent *, unsigned short);
extern void func_8003CAEC(void *, int, int, int);
extern void func_8003C5D8(void *, int);
extern void func_8006F39C(int);
extern void func_8006DE80(int, int, int, int, int);
extern void func_800293F4(int);
extern void func_800523F8(int *, int *);
extern void func_8002F300(void);

int func_8002AA98(void)
{
    int r;
    int v;
    Ent *e;

    r = 0;
    switch (D_8009CE74) {
    case 0:
        if (D_8009D254->fE != 0x13) {
            func_8001A680(D_8009D254, 0x13);
        }
        e = D_8009D254;
        if (e->fF != *(unsigned short *)((char *)e + 0x16)) {
            goto clr;
        }
        D_8009CE70 = 0x10;
        e->f98 |= 0x100;
        D_8009CE74++;
        break;
    case 1:
        v = D_8009CE70;
        if (v != 0) {
            v <<= 3;
            v = ~v;
            goto fade;
        }
        func_8003CAEC(D_8009D254->sub, 0xFF, 0xFF, 0xFF);
        func_8003CAEC(D_800B0B38.sub, 0xFF, 0xFF, 0xFF);
        func_8003C5D8(D_8009D254->sub, 0x1E);
        D_8009D254->f250 |= 2;
        func_8003C5D8(D_800B0B38.sub, 0x1E);
        D_800B0B38.f250 |= 2;
        D_8009CE74++;
        break;
    case 2: {
        register Ent *a asm("$5") = D_8009D254;

        if (a->f252 != 0) {
            break;
        }
    }
        if (D_800B0B38.f252 != 0) {
            break;
        }
        func_8006F39C(0x69);
        func_8006DE80(0x4AF, 0, D_8009D254->f2A, D_8009D254->f2E, D_8009D254->f32);
        D_8009CE70 = 0x34;
        D_8009CE74++;
        break;
    case 3:
        if (D_8009CE70 != 0) {
            D_8009CE70--;
            break;
        }
        D_8009D254->f252 = 1;
        func_8003C5D8(D_8009D254->sub, 0x1E);
        D_8009D254->f250 |= 4;
        func_8003CAEC(D_8009D254->sub, 0xFF, 0xFF, 0xFF);
        D_800B0B38.f252 = 1;
        func_8003C5D8(D_800B0B38.sub, 0x1E);
        D_800B0B38.f250 |= 4;
        func_8003CAEC(D_800B0B38.sub, 0xFF, 0xFF, 0xFF);
        func_8001A680(D_8009D254, 0xF);
        D_8009CE70 = 0x1E;
        D_8009D254->f14 = 0x230000;
        D_8009D254->f18_pad = 0x220000;
        D_8009CE74++;
        break;
    case 4:
        if (D_8009CE70 == 0x10) {
            D_8009CE74++;
            e = D_8009D254;
        clr:
            e->f98 &= ~0x100;
            break;
        }
        D_8009CE70--;
        break;
    case 5:
        v = D_8009CE70;
        if (v != 0) {
            v <<= 3;
            v += 0x7F;
        fade:
            v &= 0xFF;
            func_8003CAEC(D_8009D254->sub, v, v, v);
            func_8003CAEC(D_800B0B38.sub, v, v, v);
            D_8009CE70--;
            break;
        }
        D_8009CE74++;
        D_8009D254->f250 |= 0x20;
        break;
    case 6:
        if (D_8009D254->fF != *(unsigned short *)((char *)D_8009D254 + 0x1A)) {
            break;
        }
        func_8001A680(D_8009D254, D_8009D278->f12);
        D_8009CE74++;
        break;
    case 7:
        D_8009D278->fC = (short)D_8009D278->f1C >> 1;
        D_8009D278->f8 >>= 2;
        func_800293F4(1);
        D_8009D278->f58 = D_8009D278->fC;
        D_8009D278->f5A = D_8009D254->f210;
        D_8009D278->f5C = D_8009D254->f212;
        D_8009D278->f5E = 0x1E;
        D_8009D278->f5F = 1;
        func_800523F8(&D_8009D278->f2C, &D_8009D278->f30);
        D_800B00E8[0].r0 = 0;
        D_800B00E8[0].g0 = 0x46;
        D_800B00E8[0].b0 = 0x82;
        D_800B00E8[0].r1 = 0x9F;
        D_800B00E8[0].g1 = 0xFF;
        D_800B00E8[0].b1 = 0xF9;
        D_800B00E8[0].r2 = 0;
        D_800B00E8[0].g2 = 0x46;
        D_800B00E8[0].b2 = 0x82;
        D_800B00E8[0].r3 = 0x9F;
        D_800B00E8[0].g3 = 0xFF;
        D_800B00E8[0].b3 = 0xF9;
        D_800B6920[0].sp.r0 = 0x9F;
        D_800B6920[0].sp.g0 = 0xFF;
        D_800B6920[0].sp.b0 = 0xF9;
        D_800B00E8[1].r0 = 0;
        D_800B00E8[1].g0 = 0x46;
        D_800B00E8[1].b0 = 0x82;
        D_800B00E8[1].r1 = 0x9F;
        D_800B00E8[1].g1 = 0xFF;
        D_800B00E8[1].b1 = 0xF9;
        D_800B00E8[1].r2 = 0;
        D_800B00E8[1].g2 = 0x46;
        D_800B00E8[1].b2 = 0x82;
        D_800B00E8[1].r3 = 0x9F;
        D_800B00E8[1].g3 = 0xFF;
        D_800B00E8[1].b3 = 0xF9;
        D_800B6920[1].sp.r0 = 0x9F;
        D_800B6920[1].sp.g0 = 0xFF;
        D_800B6920[1].sp.b0 = 0xF9;
        D_800B0130[0][0].r0 = 0;
        D_800B0130[0][0].g0 = 0x82;
        D_800B0130[0][0].b0 = 0x36;
        D_800B0130[0][0].r1 = 0x4A;
        D_800B0130[0][0].g1 = 0xFF;
        D_800B0130[0][0].b1 = 0x3B;
        D_800B0130[0][0].r2 = 0;
        D_800B0130[0][0].g2 = 0x82;
        D_800B0130[0][0].b2 = 0x36;
        D_800B0130[0][0].r3 = 0x4A;
        D_800B0130[0][0].g3 = 0xFF;
        D_800B0130[0][0].b3 = 0x3B;
        D_800B0130[1][0].r0 = 0;
        D_800B0130[1][0].g0 = 0x82;
        D_800B0130[1][0].b0 = 0x36;
        D_800B0130[1][0].r1 = 0x4A;
        D_800B0130[1][0].g1 = 0xFF;
        D_800B0130[1][0].b1 = 0x3B;
        D_800B0130[1][0].r2 = 0;
        D_800B0130[1][0].g2 = 0x82;
        D_800B0130[1][0].b2 = 0x36;
        D_800B0130[1][0].r3 = 0x4A;
        D_800B0130[1][0].g3 = 0xFF;
        D_800B0130[1][0].b3 = 0x3B;
        D_8009CE74 = 0;
        D_8009D28C = 0;
        if (D_8009D2A0 != 0) {
            r = 1;
        } else {
            r = 1;
            func_8002F300();
        }
        break;
    }
    return r;
}
