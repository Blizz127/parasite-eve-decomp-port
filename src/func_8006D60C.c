typedef struct {
    unsigned int flags;
    unsigned char pad4[0xD6];
    signed char bDA;
    unsigned char padDB;
    signed char bDC;
    unsigned char padDD[3];
    signed char bE0;
    signed char bE1;
    unsigned char padE2[6];
    short hE8;
    unsigned char bEA[8];
    unsigned char state;
    unsigned char padF3[0xB];
    unsigned char bFE;
    unsigned char padFF[0x25];
    int f124;
    int f128;
    unsigned char pad12C[0x68];
    int f194;
} Scene;

extern Scene D_800B0CD8[];
extern int volatile D_800B0E08;
extern int D_8009CDA4;
extern int D_8009D188;
extern int D_8009D18C;
extern int D_8009D190;

extern void func_80087024(void);
extern void func_8006DF50(int, int, int, int, int);
extern void func_80086C5C(int, int, int);
extern int func_8006D078(void);
extern void func_800864CC(void);
extern void func_80086FF8(void);
extern int func_8006CDA4(int, int, int, int, int, int);
extern int func_80086464(int);
extern void func_80086C1C(int, int);
extern int func_8006D2B8(int, int, int, int *, int);
extern void func_8006DB48(int, int, int, int);

int func_8006D60C(int arg)
{
    Scene *s;
    int d;
    int h;
    unsigned int fl;
    int buf[2];

    s = D_800B0CD8;
loop:
    switch (s->state) {
    case 0:
        D_8009D188 = 0;
        func_80087024();
        if (arg != 0) {
            if (!(D_800B0CD8[0].flags & 0x400000) && D_800B0E08 != 0) {
                func_8006DF50(D_800B0E08, 0x451, 0, 0x80, 0x7F);
                if (D_800B0E08 != 0) {
                    func_8006DF50(D_800B0E08, 0x452, 0, 0x80, 0x7F);
                }
            }
            s->state = 0x2C;
            goto loop;
        }
        if (s->flags & 4) {
            D_8009D18C = D_8009CDA4;
            func_80086C5C(0, 0x3C, 0);
        }
        s->state = 0x2D;
        goto loop;
    case 0x2C:
        if (s->bE0 != s->bDC) {
            if (s->flags & 0x40) {
                D_8009D18C = D_8009CDA4;
                func_80086C5C(0, 0x3C, 0);
            }
            s->flags |= 4;
        }
        s->state = 0x2E;
        goto loop;
    case 0x2E:
        if (func_8006D078() == 1) {
            return 1;
        }
        fl = s->flags;
        if (fl & 4) {
            if (fl & 0x40) {
                d = 0x3C - (D_8009CDA4 - D_8009D18C);
                D_8009D190 = d;
                if (d >= 9) {
                    D_8009D190 = 8;
                    func_80086C5C(0, 0x10, 0);
                } else if (d < 0) {
                    D_8009D190 = 0;
                }
            }
        }
        s->state = 0x3F;
        goto loop;
    case 0x3F:
        if (!(s->flags & 4)) {
            s->state = 0x30;
            goto loop;
        }
        if (s->flags & 0x40) {
            if (D_8009D190 > 0) {
                D_8009D190--;
                return 1;
            }
            func_800864CC();
            func_80086FF8();
        }
        s->state = 0x2F;
        goto loop;
    case 0x2F:
        if (func_8006CDA4(0, s->bE1, 0, s->f194, 0x21, 0) == 1) {
            return 1;
        }
        s->state = 0x30;
        goto loop;
    case 0x30:
        func_80086464(s->f124);
        func_80086C1C(0, 0x7F);
        s->state = 0;
        return 0;
    case 0x2D:
        if (D_8009D188 < 2) {
            if (s->bEA[D_8009D188] != 0) {
                s->state = 0x31;
                goto loop;
            }
            D_8009D188++;
        }
        s->state = 0x32;
        goto loop;
    case 0x31:
        if (func_8006CDA4(2, s->bEA[D_8009D188], D_8009D188, s->f194, 0x21, 0) == 1) {
            return 1;
        }
        s->state = 0x2D;
        D_8009D188++;
        goto loop;
    case 0x32:
        if (s->hE8 != -1) {
            if (func_8006CDA4(1, s->hE8, 0, s->f194, 0x21, 0) == 1) {
                return 1;
            }
        }
        if (s->flags & 4) {
            d = 0x3C - (D_8009CDA4 - D_8009D18C);
            D_8009D190 = d;
            if (d >= 9) {
                D_8009D190 = 8;
                func_80086C5C(0, 0x10, 0);
            } else if (d < 0) {
                D_8009D190 = 0;
            }
        }
        s->state = 0x40;
        goto loop;
    case 0x40:
        if (s->flags & 4) {
            if (D_8009D190 > 0) {
                D_8009D190--;
                return 1;
            }
            func_80086FF8();
            if (s->flags & 0x40) {
                s->flags &= ~0x40;
                s->state = 0x3E;
                goto loop;
            }
        }
        s->state = 0;
        s->flags &= ~4;
        return 0;
    case 0x3E:
        if (func_8006CDA4(0, s->bDA, 0, s->f194, 0x21, 0) == 1) {
            return 1;
        }
        s->state = 0x33;
        goto loop;
    case 0x33:
        if (func_8006D2B8(s->bDC, 1, 0, buf, 0) == 1) {
            return 1;
        }
        h = func_80086464(s->f128);
        if (h == -1) {
            s->state = 0x3E;
            return 1;
        }
        func_80086C5C(h, 0x3C, s->bFE);
        if (h != 0) {
            func_8006DB48(0, s->bDC, h, s->bFE);
        }
        s->state = 0;
        s->flags = (s->flags & ~4) | 0x40;
        return 0;
    }
    return 0;
}
