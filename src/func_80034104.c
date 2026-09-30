typedef struct Sprt {
    unsigned int tag;
    unsigned char r0, g0, b0, code;
    short x0, y0;
    unsigned char u0, v0;
    unsigned short clut;
    short w, h;
} Sprt;

typedef struct Ent {
    unsigned int tp[2];
    Sprt sp;
} Ent;

typedef struct Rec {
    unsigned char pad0[0x4C];
    unsigned int f4C;
} Rec;

extern Rec **D_8009D254;
extern int D_8009D1E8;
extern int D_8009CDDC;
extern unsigned short D_8009CE84[2];
extern Ent D_8009E1D0[][5];
extern Ent D_8009E0F0[][4];
extern Ent D_8009E2E8[];
extern unsigned char *D_800B0E38[];
extern void func_80077AC4(unsigned char *, Ent *);

void func_80034104(short hp, short maxhp)
{
    char buf[8];
    signed char n;
    signed char cnt;
    signed char d;
    short x;
    short y;
    short q;
    unsigned char t;
    unsigned int u;

    n = 0;
    y = D_8009CE84[1] + 0xB;
    while (q = hp / 10, buf[n] = hp - q * 10, (hp = q) != 0) {
        n++;
    }
    n++;
    buf[n] = 10;
    cnt = n;
    x = D_8009CE84[0] - n * 6 + 0x38;
    for (; n >= 0; n--) {
        (&D_8009E1D0[D_8009CDDC][n].sp)->u0 = buf[n] * 8;
        (&D_8009E1D0[D_8009CDDC][n].sp)->v0 = 0xE8;
        (&D_8009E1D0[D_8009CDDC][n].sp)->x0 = x;
        (&D_8009E1D0[D_8009CDDC][n].sp)->y0 = y;
        if (!((*D_8009D254)->f4C & 0x800) || n == cnt) {
            (&D_8009E1D0[D_8009CDDC][n].sp)->r0 = 0x80;
            (&D_8009E1D0[D_8009CDDC][n].sp)->g0 = 0x80;
            (&D_8009E1D0[D_8009CDDC][n].sp)->b0 = 0x80;
        } else {
            u = D_8009D1E8 & 0x1F;
            t = u;
            if (t > 16) {
                t = 32 - u;
            }
            (&D_8009E1D0[D_8009CDDC][n].sp)->r0 = t * 6 + 0x20;
            (&D_8009E1D0[D_8009CDDC][n].sp)->g0 = t + 0x60;
            (&D_8009E1D0[D_8009CDDC][n].sp)->b0 = 0x80;
        }
        x += 6;
        func_80077AC4(D_800B0E38[D_8009CDDC] + 0x14, &D_8009E1D0[D_8009CDDC][n]);
    }
    n = 0;
    x = D_8009CE84[0] - (cnt * 2 + 1) * 6 + 0x3E;
    while (q = maxhp / 10, buf[n] = maxhp - q * 10, (maxhp = q) != 0) {
        n++;
    }
    d = cnt - n - 1;
    if (d > 0) {
        x += d * 6;
    }
    for (; n >= 0; n--) {
        (&D_8009E0F0[D_8009CDDC][n].sp)->u0 = buf[n] * 8;
        (&D_8009E0F0[D_8009CDDC][n].sp)->v0 = 0xE8;
        (&D_8009E0F0[D_8009CDDC][n].sp)->x0 = x;
        (&D_8009E0F0[D_8009CDDC][n].sp)->y0 = y;
        if ((*D_8009D254)->f4C & 0x400) {
            u = D_8009D1E8 & 0x1F;
            t = u;
            if (t > 16) {
                t = 32 - u;
            }
            (&D_8009E0F0[D_8009CDDC][n].sp)->r0 = t * 8;
            (&D_8009E0F0[D_8009CDDC][n].sp)->g0 = 0x80;
            (&D_8009E0F0[D_8009CDDC][n].sp)->b0 = t * 6 + 0x20;
        } else {
            (&D_8009E0F0[D_8009CDDC][n].sp)->r0 = 0x80;
            (&D_8009E0F0[D_8009CDDC][n].sp)->g0 = 0x80;
            (&D_8009E0F0[D_8009CDDC][n].sp)->b0 = 0x80;
        }
        x += 6;
        func_80077AC4(D_800B0E38[D_8009CDDC] + 0x14, &D_8009E0F0[D_8009CDDC][n]);
    }
    (&D_8009E2E8[D_8009CDDC].sp)->x0 = D_8009CE84[0] + 0x44;
    (&D_8009E2E8[D_8009CDDC].sp)->y0 = D_8009CE84[1] + 0xE;
    func_80077AC4(D_800B0E38[D_8009CDDC] + 0x10, &D_8009E2E8[D_8009CDDC]);
}
