typedef struct Pl {
    int exp;
    unsigned char pad4[2];
    unsigned short h06;
    unsigned char pad8[2];
    unsigned char b0A;
    unsigned char padB[5];
    int w10;
    unsigned char pad14[0xA];
    unsigned short h1E;
    unsigned char pad20[8];
    short h28[7];
} Pl;

extern Pl D_800C0E00;
extern int D_8009CFE8;
extern int D_8009CFEC;
extern int D_8009CFF0;
extern int D_8009CEFC;
extern int D_8009CF60;
extern int D_8009CF64;
extern int D_8009CF68;
extern int D_8009CF6C;
extern int D_8009CF70;
extern int D_8009CF74;
extern int D_8009CF84;
extern int D_800A18B4[];
extern int D_800A18D8[];
extern int D_800A18FC[];
extern void func_8005B890(int);
extern void func_80051510(void);
extern int *func_8005DBF8(void);
extern int func_8005B8A8(int, int *);
extern void func_8005B91C(int, int, int *, int);
extern int func_80051DF8(int);
extern void func_80057E14(int);
extern void func_8004B90C(void);
extern void func_8005270C(void);
extern void func_8005C144(void);

static inline int clamp_max(int v, int max)
{
    return v > max ? max : v;
}

void func_8004B70C(int gain, int ap, int a2)
{
    Pl *p;
    int i;
    int t;
    int tmp;

    func_8005B890(0);
    func_80051510();
    p = &D_800C0E00;
    D_8009CFE8 = p->exp;
    D_8009CFEC = p->exp + gain;
    D_8009CFF0 = func_8005B8A8(D_8009CFE8, func_8005DBF8());
    D_8009CEFC = 1;
    D_800C0E00.h1E += ap;
    D_8009CF60 = D_800C0E00.b0A;
    D_8009CF64 = D_800C0E00.h06;
    D_8009CF68 = D_8009CF74 = D_800C0E00.w10;
    D_8009CF6C = func_8005B8A8(D_8009CFEC, func_8005DBF8());
    if (D_8009CF60 < D_8009CF6C) {
        D_8009CF74 = clamp_max(D_8009CF74 + D_800C0E00.h1E, 99999);
        asm("" : "=r"(p) : "0"(p));
        p->h1E = 0;
    }
    {
        short *hp = D_800C0E00.h28;
        for (i = 0; i < 7; i++) {
            t = *hp++;
            D_800A18D8[i] = t;
            func_8005B91C(i, t, &D_800A18B4[i], 0);
            D_800A18FC[i] = (D_8009CF6C - D_8009CF60) * 10;
        }
    }
    func_8005B91C(0, D_800A18D8[0] + D_800A18FC[0], &tmp, 0);
    D_8009CF70 = func_80051DF8(tmp);
    D_8009CF84 = 2;
    func_80057E14(a2);
    func_8004B90C();
    func_8005270C();
    func_8005C144();
}
