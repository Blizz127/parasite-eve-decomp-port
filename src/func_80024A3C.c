typedef struct {
    unsigned char *p;
    short id;
    short pad;
} Ent;

typedef struct {
    int x;
    int y;
    int z;
} Vec;

typedef struct {
    unsigned char pad[0x9C];
    unsigned short f9C;
    unsigned char f9E;
} Obj;

typedef struct {
    unsigned int lo : 13;
    unsigned int a : 2;
    unsigned int c : 3;
    unsigned int b : 2;
    unsigned int hi : 12;
} Bits;

typedef struct {
    unsigned int lo : 20;
    unsigned int v : 2;
    unsigned int hi : 10;
} Bits2;

typedef struct { int f; } SW;
typedef struct { short f; } SH;
typedef struct { unsigned short f; } SUH;
typedef struct { unsigned char f; } SB;
#define W(p, o) (((SW *)((unsigned char *)(p) + (o)))->f)
#define H(p, o) (((SH *)((unsigned char *)(p) + (o)))->f)
#define UH(p, o) (((SUH *)((unsigned char *)(p) + (o)))->f)
#define PW(p, o) (*(int *)((unsigned char *)(p) + (o)))
#define PUW(p, o) (*(unsigned int *)((unsigned char *)(p) + (o)))
#define PUH(p, o) (*(unsigned short *)((unsigned char *)(p) + (o)))
#define B(p, o) (((SB *)((unsigned char *)(p) + (o)))->f)

extern Ent D_800BE830[];
extern unsigned char D_8009D25C;
extern int D_8009D258;
extern unsigned short D_8009CE58[3];
extern signed char D_8009CE48;
extern short D_8009CE4C;
extern unsigned char D_8009D1D4;
extern unsigned char D_8009CE54;
extern signed char D_8009CE55;
extern unsigned char *D_8009D254;
extern unsigned char *D_8009D278;
extern unsigned char *D_8009D20C;
extern unsigned int D_8009D2E8;
extern unsigned int D_8009D1A0;
extern Vec D_8009E054;
extern Obj D_800B0CEC;
extern short D_801F1F38;

extern int func_8006C1CC(int);
extern void func_8003C5D8(void *, int);
extern void func_800702DC(void);
extern int func_8006F39C(int, unsigned char *);
extern void func_8001A680(unsigned char *, unsigned short);
extern void func_8006FC18(int, unsigned char *, int);
extern int func_80030584(unsigned char *, Vec *);
extern int func_80077CF4(short);
extern int func_80077DC4(short);
extern void func_8006DE80(int, int, int, int, int);

signed char func_80024A3C(void)
{
    signed char r;
    unsigned char *e;
    int s;
    int t;
    int k;

    r = 0;
    switch (D_8009D25C) {
    case 0:
        {
        unsigned char *p;
        func_8006C1CC(1);
        p = D_8009D254;
        D_8009E054.x = PW(p, 0x28);
        D_8009E054.y = PW(p, 0x2C);
        D_8009E054.z = PW(p, 0x30);
        D_8009CE58[0] = PUH(p, 0x38);
        D_8009CE58[1] = PUH(p, 0x3A);
        D_8009CE48 = 0;
        D_8009CE58[2] = PUH(p, 0x3C);
        D_8009D2E8 &= ~4;
        W(p, 0x98) |= 0x80;
        func_8003C5D8(p + 0x1B4, 0x1E);
        UH(D_8009D254, 0x250) |= 2;
        func_8003C5D8(&D_800B0CEC, 0x1E);
        D_800B0CEC.f9C |= 2;
        func_800702DC();
        D_8009D258 = func_8006F39C(0x6B, D_8009D254);
        D_8009D25C++;
        break;
        }
    case 1:
        func_8006C1CC(1);
        if (B(D_8009D254, 0x252) != 0) {
            break;
        }
        if (D_800B0CEC.f9E != 0) {
            break;
        }
        D_8009D25C++;
        break;
    case 2:
        if (func_8006C1CC(1) != 0) {
            break;
        }
        func_8001A680(D_8009D254, 5);
        {
        unsigned char *p = D_8009D254;
        *(p + 0x252) = 1;
        W(p, 0x98) |= 0x100;
        }
        func_8003C5D8(D_8009D254 + 0x1B4, 0x1E);
        UH(D_8009D254, 0x250) |= 4;
        func_8006F39C(0x6C, D_8009D254);
        D_8009CE4C = 0x1E;
        D_8009D25C++;
        break;
    case 3:
        if (D_8009CE4C == 0) {
            D_8009D25C++;
            W(D_8009D254, 0x98) &= ~0x100;
        } else {
            D_8009CE4C--;
        }
    case 4:
        {
        unsigned char *p = D_8009D254;
        if (B(p, 0xF) != UH(p, 0x16)) {
            break;
        }
        {
        unsigned char *q;
        for (q = D_8009D20C; q != 0; q = (unsigned char *)W(q, 4)) {
            if (q != p && W(q, 0) != 0) {
                W(q, 0x68) = 0;
                W(q, 0x6C) = 0;
                W(q, 0x70) = 0;
            }
        }
        }
        func_8006FC18(D_8009D258, D_8009D254, 0);
        func_8006F39C(0x6D, D_8009D254);
        D_8009D25C++;
        break;
        }
    case 5:
        func_8001A680(D_8009D254, 6);
        func_8003C5D8(D_8009D254 + 0x1B4, 0xF);
        D_8009D25C++;
        UH(D_8009D254, 0x250) |= 2;
        break;
    case 6:
        {
        unsigned char *p = D_8009D254;
        if (B(p, 0x252) != 0) {
            break;
        }
        e = D_800BE830[D_8009D1D4].p;
        s = H(e, 0x224) * (int)((PUW(W(e, 0), 0xCC) >> 19) & 0x1F) / 10 + H(p, 0x224);
        t = func_80030584(e + 0x1B4, &D_8009E054);
        H(D_8009D254, 0x3A) = t;
        t = func_80077CF4(t);
        W(D_8009D254, 0x28) = (H(D_800BE830[D_8009D1D4].p, 0x268) << 16) + ((s * t) << 4);
        t = func_80077DC4(H(D_8009D254, 0x3A));
        {
        unsigned char *q = D_8009D254;
        W(q, 0x30) = (H(D_800BE830[D_8009D1D4].p, 0x26C) << 16) + ((s * t) << 4);
        W(q, 0x40) = W(q, 0x28);
        W(q, 0x44) = W(q, 0x2C);
        W(q, 0x48) = W(q, 0x30);
        func_8006DE80(0x4B6, 0, H(q, 0x2A), H(q, 0x2E), H(q, 0x32));
        }
        D_8009CE4C = 0x1E;
        D_8009D25C++;
        break;
        }
    case 7:
        {
        unsigned char *p = D_8009D254;
        if (B(p, 0xF) == UH(p, 0x1A)) {
            func_8001A680(p, 7);
            W(D_8009D254, 0x14) = B(D_8009D254, 0xF) << 15;
        }
        if (D_8009CE4C != 0) {
            D_8009CE4C--;
            break;
        }
        B(D_8009D254, 0x252) = 1;
        func_8003C5D8(D_8009D254 + 0x1B4, 0xF);
        D_8009D25C++;
        UH(D_8009D254, 0x250) |= 4;
        break;
        }
    case 8:
        {
        unsigned char *p = D_8009D254;
        if (B(p, 0xF) != UH(p, 0x1A)) {
            break;
        }
        {
        unsigned short v = UH(p, 0x250) | 0x20;
        unsigned short a = D_8009CE48 * 2 + 8;
        UH(p, 0x250) = v;
        func_8001A680(p, a);
        }
        D_8009D25C++;
        break;
        }
    case 9:
        {
        unsigned char *p = D_8009D254;
        if (B(p, 0xF) != UH(p, 0x1A)) {
            break;
        }
        func_8001A680(p, D_8009CE48 * 2 + 9);
        k = D_8009D1D4;
        D_8009CE55 = 2;
        D_8009CE54 = 1;
        {
        unsigned char *q = (unsigned char *)W(*(unsigned char **)((unsigned char *)D_800BE830 + k * 8), 0);
        ((Bits *)q)->a = 1;
        PW(q, 0) = (PW(q, 0) & 0xFFF3FFFF) | (((PUW(W(D_8009D278, 0x68), 0xC) >> 20) & 3) << 18);
        ((Bits *)q)->c = D_8009CE55;
        }
        if (D_8009CE48 == 2) {
            func_8006F39C(0x6F, D_800BE830[k].p);
        } else if (D_8009CE48 == 5) {
            func_8006F39C(0x71, D_800BE830[k].p);
        } else if (D_8009CE48 == 6) {
            func_8006F39C(0x70, D_800BE830[k].p);
        } else {
            func_8006F39C(0x6E, D_800BE830[k].p);
        }
        D_8009CE48++;
        if (D_8009CE48 < 7 && D_800BE830[D_8009D1D4].p == D_800BE830[D_8009D1D4 + 1].p) {
            D_8009D25C = 8;
            D_8009D1D4++;
            break;
        }
        D_8009D25C++;
        break;
        }
    case 10:
        {
        unsigned char *p = D_8009D254;
        if (B(p, 0xF) != UH(p, 0x1A)) {
            break;
        }
        func_8001A680(p, 7);
        if (D_8009CE48 < 7) {
            D_8009D25C = 5;
            D_8009D1D4++;
            break;
        }
        func_8003C5D8(D_8009D254 + 0x1B4, 0xF);
        D_8009D25C++;
        UH(D_8009D254, 0x250) |= 2;
        break;
        }
    case 11:
        {
        unsigned char *q = D_8009D254;
        if (B(q, 0x252) != 0) {
            break;
        }
        PW(q, 0x28) = D_8009E054.x;
        PW(q, 0x2C) = D_8009E054.y;
        PW(q, 0x30) = D_8009E054.z;
        PW(q, 0x40) = D_8009E054.x;
        PW(q, 0x44) = D_8009E054.y;
        PW(q, 0x48) = D_8009E054.z;
        PUH(q, 0x38) = D_8009CE58[0];
        PUH(q, 0x3A) = D_8009CE58[1];
        PUH(q, 0x3C) = D_8009CE58[2];
        func_8006DE80(0x4B6, 0, H(q, 0x2A), H(q, 0x2E), H(q, 0x32));
        D_8009CE4C = 0x1E;
        D_8009D25C++;
        break;
        }
    case 12:
        if (D_8009CE4C != 0) {
            D_8009CE4C--;
            break;
        }
        func_8001A680(D_8009D254, 7);
        B(D_8009D254, 0x252) = 1;
        func_8003C5D8(D_8009D254 + 0x1B4, 0x1E);
        D_801F1F38 = 1;
        D_8009CE4C = 0xF;
        D_8009D25C++;
        UH(D_8009D254, 0x250) |= 4;
        break;
    case 13:
        {
        unsigned char *p;
        if (D_8009CE4C != 0) {
            D_8009CE4C--;
            break;
        }
        p = D_8009D254;
        if (B(p, 0xF) != UH(p, 0x1A)) {
            break;
        }
        func_8001A680(p, 4);
        D_8009D25C++;
        break;
        }
    case 14:
        {
        unsigned char *p = D_8009D254;
        if (B(p, 0xF) != UH(p, 0x16)) {
            break;
        }
        W(p, 0x98) |= 0x100;
        func_8003C5D8(p + 0x1B4, 0xF);
        UH(D_8009D254, 0x250) |= 2;
        func_8006F39C(0x72, D_8009D254);
        D_8009D25C++;
        break;
        }
    case 15:
        {
        unsigned char *p;
        if (B(D_8009D254, 0x252) != 0) {
            break;
        }
        if (func_8006C1CC(0) != 0) {
            break;
        }
        func_8001A680(D_8009D254, D_8009D278[0x12]);
        D_8009D2E8 |= 4;
        p = D_8009D254;
        B(p, 0x252) = 1;
        W(p, 0x98) &= ~0x100;
        W(p, 0x98) &= ~0x80;
        func_8003C5D8(D_8009D254 + 0x1B4, 0x1E);
        UH(D_8009D254, 0x250) |= 4;
        D_800B0CEC.f9E = 1;
        func_8003C5D8(&D_800B0CEC, 0x1E);
        D_8009CE4C = 0xF;
        D_800B0CEC.f9C |= 4;
        D_8009D25C++;
        break;
        }
    case 16:
        if (D_8009CE4C == 0) {
            D_8009D1A0 &= ~0x100;
            D_8009D1D4++;
            r = 1;
        } else {
            D_8009CE4C--;
        }
        break;
    }
    return r;
}
