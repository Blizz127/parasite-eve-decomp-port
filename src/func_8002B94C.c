typedef struct Sub {
    unsigned char pad0[6];
    signed char f6;
    unsigned char pad7[0xA8];
    unsigned char fAF;
} Sub;

typedef struct Actor {
    Sub *f0;
    struct Actor *next;
    unsigned char pad8[0x60];
    int f68;
    int f6C;
    int f70;
    unsigned char pad74[0x24];
    unsigned int f98;
    unsigned char pad9C[0xF0];
    void *f18C;
    unsigned char pad190[0x24];
    unsigned char f1B4[0x9C];
    unsigned short f250;
    unsigned char f252;
} Actor;

extern Actor *D_8009D20C;
extern Actor *D_8009D254;
extern unsigned char D_8009CE74;
extern unsigned char D_8009D244;
extern int D_8009D28C;
typedef struct { unsigned short v; } H16;
typedef struct { unsigned char v; } H8;
extern H16 D_800B0D88;
extern H8 D_800B0D8A;
extern unsigned char D_800B0CEC[];
extern void func_800701B4(void);
extern void func_8003C5D8(unsigned char *, int);
extern void func_8001A680(Actor *, unsigned short);
extern void func_800293F4(int);
extern void func_800866A4(int, int);
extern void func_800703F4(void);
extern int func_8006D60C(int);
extern void func_8002F9CC(void);
extern void func_800295E4(void);

void func_8002B94C(void)
{
    Actor *t;
    unsigned char ok;

    switch (D_8009CE74) {
    case 0:
        D_8009D244 = 0;
        func_800701B4();
        for (t = D_8009D20C; t != 0; t = t->next) {
            if (t->f0 == 0) {
                if (t->f18C == 0) {
                    continue;
                }
            } else if (t == D_8009D254) {
                func_8003C5D8(D_800B0CEC, 0x1E);
                D_800B0D88.v |= 2;
                D_8009D254->f250 |= 2;
            } else {
                func_8001A680(t, t->f0->f6);
                t->f98 |= 0x1000;
                t->f68 = 0;
                t->f6C = 0;
                t->f70 = 0;
                if ((t->f98 & 0x40000000) && t->f0->fAF == 0) {
                    t->f98 |= 0x10;
                    t->f0 = 0;
                }
            }
            func_8003C5D8(t->f1B4, 0x1E);
        }
        D_8009CE74++;
        break;
    case 1:
        if (D_8009D254->f252 == 0 && D_800B0D8A.v == 0) {
            func_800293F4(0);
            D_8009CE74++;
            for (t = D_8009D20C; t != 0; t = t->next) {
                if (t != D_8009D254 && (t->f0 != 0 || t->f18C != 0)) {
                    t->f250 |= 2;
                }
            }
        }
        break;
    case 2:
        ok = 1;
        for (t = D_8009D20C; t != 0; t = t->next) {
            if (t != D_8009D254 && (t->f0 != 0 || t->f18C != 0)) {
                if (t->f252 == 0) {
                    t->f0 = 0;
                    t->f98 |= 0x10;
                } else if (t->f98 & 0x40) {
                    t->f98 |= 0x10;
                    t->f0 = 0;
                } else {
                    ok = 0;
                }
            }
        }
        if (ok) {
            func_800866A4(0, 0xFF);
            func_800703F4();
            D_8009CE74++;
        }
        break;
    case 3:
        if (func_8006D60C(0) != 1) {
            func_8002F9CC();
            func_8001A680(D_8009D254, 0x15);
            func_800295E4();
            D_8009D28C = 10;
        }
        break;
    }
}
