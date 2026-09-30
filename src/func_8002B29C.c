typedef struct Rec {
    unsigned char pad0[5];
    signed char f5;
    signed char f6;
    unsigned char pad7[0xAF - 7];
    unsigned char fAF;
} Rec;

typedef struct Ent {
    Rec *f0;
    struct Ent *next;
    unsigned char pad8[0xE - 8];
    unsigned char fE;
    unsigned char fF;
    unsigned char pad10[0x68 - 0x10];
    int f68;
    int f6C;
    int f70;
    unsigned char pad74[0x98 - 0x74];
    int f98;
    unsigned char pad9C[0x1B4 - 0x9C];
    unsigned char sub[0x250 - 0x1B4];
    unsigned short f250;
    unsigned char f252;
} Ent;

typedef struct PolyFT4 {
    unsigned int tag;
    unsigned char r0, g0, b0, code;
    short x0, y0;
    unsigned char u0, v0;
    unsigned short clut;
    short x1, y1;
    unsigned char u1, v1;
    unsigned short tpage;
    short x2, y2;
    unsigned char u2, v2;
    unsigned short pad2;
    short x3, y3;
    unsigned char u3, v3;
    unsigned short pad3;
} PolyFT4;

extern Ent *D_8009D254;
extern Ent *D_8009D20C;
extern Ent D_800B0B38;
extern int D_8009CDDC;
extern unsigned char D_8009CE70;
extern unsigned char D_8009CE74;
extern int D_8009D28C;
extern PolyFT4 D_800BE9F0[];
extern unsigned char *D_800B0E38[];

extern void func_8001A680(Ent *, unsigned short);
extern void func_800293F4(int);
extern void func_80067B40(int);
extern void func_80086C5C(int, int, int);
extern void func_8003C5D8(void *, int);
extern void func_800703F4(void);
extern void func_800866A4(int, int);
extern unsigned int func_80077DC4(int);
extern void func_800295E4(void);
extern void func_8006A25C(void);
extern void func_80077AC4(void *, void *);

#define setXYWH(p, _x0, _y0, _w, _h) \
    (p)->x0 = (p)->x2 = (_x0), (p)->y0 = (p)->y1 = (_y0), \
    (p)->x1 = (p)->x3 = (_x0) + (_w), (p)->y2 = (p)->y3 = (_y0) + (_h)
#define setRGB0(p, _r0, _g0, _b0) \
    (p)->r0 = (_r0), (p)->g0 = (_g0), (p)->b0 = (_b0)
#define Q (&D_800BE9F0[D_8009CDDC])

void func_8002B29C(void)
{

    char pad[24];
    Ent *e;
    Ent *o;
    PolyFT4 *p;
    int c;
    unsigned char t;
    int a;
    int b;
    unsigned int h;

    switch (D_8009CE74) {
    case 0:
        e = D_8009D254;
        if (e->fE != 0x13) {
            func_8001A680(e, 0x13);
            e = D_8009D254;
        }
        if (e->fF != *(unsigned short *)((char *)e + 0x16)) {
            goto clr;
        }
        for (o = D_8009D20C; o != 0; o = o->next) {
            if (o == D_8009D254) {
                continue;
            }
            if (o->f0->f5 == 1) {
                continue;
            }
            if (o->f0 == 0 && (o->f98 & 0x40)) {
                continue;
            }
            o->f250 |= 2;
            o->f98 |= 0x1000;
            if (o->f0 != 0) {
                func_8001A680(o, o->f0->f6);
                if ((o->f98 & 0x40000000) && o->f0->fAF == 0) {
                    o->f98 |= 0x10;
                    o->f0 = 0;
                }
            }
            o->f68 = 0;
            o->f6C = 0;
            o->f70 = 0;
        }
        func_800293F4(0);
        e = D_8009D254;
        D_8009CE70 = 0x46;
        D_8009CE74++;
        e->f98 |= 0x100;
        break;
    clr:
        e->f98 &= ~0x100;
        break;
    case 1:
        if (D_8009CE70 == 0x3C) {
            func_80067B40(0x3C);
            func_80086C5C(0, 0x3C, 0);
            for (o = D_8009D20C; o != 0; o = o->next) {
                if (o == D_8009D254) {
                    continue;
                }
                if (o->f0->f5 == 1) {
                    continue;
                }
                if (o->f0 == 0 && (o->f98 & 0x40)) {
                    continue;
                }
                func_8003C5D8(o->sub, 0x3C);
            }
        } else {
            for (o = D_8009D20C; o != 0; o = o->next) {
                if (o == D_8009D254) {
                    continue;
                }
                if (o->f0 == 0 && (o->f98 & 0x40)) {
                    continue;
                }
                if (o->f252 != 0) {
                    continue;
                }
                o->f98 |= 0x10;
            }
        }
        if (D_8009CE70 != 0) {
            D_8009CE70--;
            break;
        }
        func_800703F4();
        func_800866A4(0, 0xFF);
        D_8009CE70 = 0x1E;
        D_8009CE74++;
        break;
    case 2: {
        register int k78 asm("$4") = 0x78;
        register int k7a asm("$5") = 0x7A;
        register PolyFT4 *b asm("$3");
        asm volatile("");
        c = k78 - D_8009CE70 * 4;
        D_800BE9F0[D_8009CDDC].r0 = c;
        D_800BE9F0[D_8009CDDC].g0 = c;
        b = D_800BE9F0;
        D_800BE9F0[D_8009CDDC].b0 = c;
        t = D_8009CE70;
        { PolyFT4 *p = &b[D_8009CDDC];
        p->x0 = 0x64 - t * 4; p->y0 = k7a; p->x1 = (0x64 - t * 4) + (0x78 + t * 8); p->y1 = k7a;
        p->x2 = 0x64 - t * 4; p->y2 = 0x7C; p->x3 = (0x64 - t * 4) + (0x78 + t * 8); p->y3 = 0x7C; }
        if (t != 0) {
            D_8009CE70 = t - 1;
            break;
        }
        D_8009CE70 = 0x50;
        D_8009CE74++;
        break; }
    case 3:
        t = D_8009CE70;
        if (t >= 0x1A) {
            PolyFT4 *b;
            h = (func_80077DC4((t - 0x10) << 4) * 11) >> 11;
            b = D_800BE9F0;
            { int k = 0x64;
            asm volatile("");
            h &= 0xFF;
            { PolyFT4 *p = &b[D_8009CDDC]; int y = 0x7A - h;
            p->x0 = k; p->y0 = y; p->x1 = 0x64 + 0x78; p->y1 = y;
            p->x2 = k; p->y2 = 0x7C; p->x3 = 0x64 + 0x78; p->y3 = 0x7C; } }
        } else {
            PolyFT4 *b = D_800BE9F0;
            PolyFT4 *p = &b[D_8009CDDC];
            p->x0 = 0x64; p->y0 = 0x64; p->x1 = 0x64 + 0x78; p->y1 = 0x64;
            p->x2 = 0x64; p->y2 = 0x7C; p->x3 = 0x64 + 0x78; p->y3 = 0x7C;
        }
        if (D_8009CE70 != 0) {
            D_8009CE70--;
            break;
        }
        func_8003C5D8(D_8009D254->sub, 0x3C);
        D_8009D254->f250 |= 2;
        func_8003C5D8(D_800B0B38.sub, 0x3C);
        D_8009CE70 = 0x3C;
        D_800B0B38.f250 |= 2;
        D_8009CE74++;
        break;
    case 4:
        if (D_8009D254->f252 == 0 && D_800B0B38.f252 == 0) {
            D_8009CE74++;
            break;
        }
        { register int c5 asm("$5") = D_8009CE70 * 2; int k220 = 0x64 + 0x78;
        PolyFT4 *p = Q;
        p->r0 = c5;
        p->x0 = 0x64; p->y0 = 0x64; p->x1 = k220; p->y1 = 0x64;
        p->x2 = 0x64; p->y2 = 0x64 + 0x18; p->x3 = k220; p->y3 = 0x64 + 0x18;
        Q->g0 = c5;
        Q->b0 = c5; }
        D_8009CE70--;
        break;
    case 5:
        func_800295E4();
        D_8009D28C = -1;
        func_8006A25C();
        break;
    }
    func_80077AC4(D_800B0E38[D_8009CDDC] + 0x10, &D_800BE9F0[D_8009CDDC]);
}
