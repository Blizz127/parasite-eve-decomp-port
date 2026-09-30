typedef void (*Op)(unsigned char *, unsigned int);

#define U8(o) (*(unsigned char *)(v + (o)))
#define U16(o) (*(unsigned short *)(v + (o)))
#define S16(o) (*(short *)(v + (o)))
#define U32(o) (*(unsigned int *)(v + (o)))
#define I32(o) (*(int *)(v + (o)))
#define PTR(o) (*(unsigned char **)(v + (o)))

typedef struct {
    unsigned int f0;
    unsigned char pad4[0xC];
    unsigned int f10;
    unsigned int f14;
    unsigned int f18;
    unsigned char pad1C[0x1C];
    unsigned int f38;
} Ctl;

extern Op D_8009CCF0[];
extern Op D_8009C8F0[];
extern Ctl *D_8009D2C8;
extern unsigned int D_800BCD5C;
extern unsigned int D_800BCD54;
extern unsigned int D_800BCD58;
extern unsigned short D_8009B8DC[];
extern unsigned char D_800B2900[];
extern int D_800B2900_w[] asm("D_800B2900");
extern int D_8009C080[];

extern int func_8008E2DC(unsigned char *);
extern void func_8008F0D0(unsigned char *, unsigned char *, int);
extern unsigned int func_8008E840(int, unsigned int, int);
extern void func_80089B28(void);
extern void func_8008E7F4(unsigned char *, unsigned int);

void func_8008E8D0(unsigned char *v, unsigned int bit)
{
    unsigned char *p;
    unsigned int op;
    unsigned int k;
    unsigned int n;
    unsigned char *ent;
    unsigned int fl;
    unsigned int x;
    unsigned int d;
    unsigned int m;
    int add;
    int add2;
    int t;
    unsigned short u;
    int pad[2];
    Ctl *c;
    unsigned int f;
    unsigned int w;
    unsigned int y;

    for (;;) {
        p = PTR(0);
        PTR(0) = p + 1;
        op = *p;
        if (op >= 0xA0) {
            if (op == 0xFC) {
                PTR(0) = p + 2;
                k = p[1];
                D_8009CCF0[k](v, bit);
            } else {
                if (op == 0xCA && (U32(0x38) & 0x200000)) {
                    D_800BCD5C |= bit;
                    op = 0xA0;
                }
                D_8009C8F0[op](v, bit);
            }
        }
        if (op < 0xA0 || op == 0xA0) {
            break;
        }
    }
    if (op == 0xA0) {
        if (U16(0x54) == 0 && (D_8009D2C8->f14 & bit) && U32(0xF0) < 0x18) {
            D_8009D2C8->f18 |= bit;
        }
        return;
    }
    k = func_8008E2DC(v) & 0xFF;
    {
        int a = S16(0xD2);
        unsigned short t2 = a;
        if (a != 0) {
            U16(0x56) = U16(0x58) = t2;
        }
    }
    if (U16(0x56) != 0) {
        if (k >= 0x8F || (k < 0x84 && !(U16(0x84) & 5))) {
            U16(0x58) -= 2;
        }
    } else {
        add = D_8009B8DC[op % 11];
        U16(0x56) = add;
        t = add;
        if (k - 0x84 >= 0xB && !(U16(0x84) & 5)) {
            t -= 2;
        }
        U16(0x58) = t;
    }
    U16(0xD0) = U16(0x56);
    U32(0xF4) |= 0x4000;
    if (k < 0x8F) {
        U32(0x38) &= ~0x40;
    } else {
        U32(0x38) |= 0x40;
    }
    if (op >= 0x8F) {
        if (U16(0x54) == 0 && (D_8009D2C8->f14 & bit) && U32(0xF0) < 0x18) {
            D_8009D2C8->f18 |= bit;
        }
        U16(0x82) = 0;
        U16(0xE8) = 0;
        U16(0xEA) = 0;
        U16(0x84) &= 0xFFFD;
        return;
    }
    if (op < 0x84) {
        op = op / 11;
        if (U32(0x38) & 8) {
            c = D_8009D2C8;
            c->f10 |= bit;
            if ((c->f14 & bit) && U32(0xF0) < 0x18) {
                c->f18 |= bit;
            }
            ent = PTR(0x14);
            ent += (op % 12) * 6;
            f = D_8009D2C8->f0 & 0x100;
            add2 = -(f != 0) & 0x30;
            w = U16(0x5A);
            y = ent[0];
            if (y < 0x20) {
                if (w == y) {
                    goto same;
                }
            } else if (w == y + add2) {
                goto reload;
            }
            {
                unsigned int b = ent[0];
                x = (unsigned char)b;
                if (b >= 0x20) {
                    x += add2;
                }
            }
            U16(0x5A) = x;
            func_8008F0D0(v, D_800B2900 + x * 64, D_800B2900_w[x * 16]);
reload:
            w = U16(0x5A);
same:
            k = func_8008E840(w, ent[1], S16(0xE0));
            { int s = U16(0x6A); s *= ent[2] + ent[3] * 256; I32(0x44) = s * 4; }
            U16(0x76) = ((ent[4] + 0x40) & 0xFF) << 8;
            if (ent[5] != 0) {
                D_8009D2C8->f38 |= bit;
            } else {
                D_8009D2C8->f38 &= ~bit;
            }
            func_80089B28();
        } else {
            op += U16(0x7C) * 12;
            if (!(U16(0x84) & 2)) {
                if (U16(0x54) == 0) {
                    if (U32(0x38) & 0x1000) {
                        func_8008E7F4(v, op);
                    }
                    c = D_8009D2C8;
                    c->f10 |= bit;
                    if ((c->f14 & bit) && U32(0xF0) < 0x18) {
                        c->f18 |= bit;
                    }
                } else {
                    D_800BCD54 |= bit;
                }
                U16(0x7A) = 0;
            }
            if (U16(0x82) != 0 && U16(0x80) != 0) {
                U16(0x7E) = U16(0x82);
                U16(0xE4) = U16(0xDE) + op - U16(0x80) - U16(0xE6);
                U16(0xE2) = U16(0x80) - (U16(0xDE) - U16(0xE6));
                op = U16(0x80) + S16(0xE6);
            } else {
                U16(0xE2) = op;
                op += S16(0xDE);
            }
            k = func_8008E840(U16(0x5A), op, S16(0xE0));
        }
        I32(0x30) = k;
        if (U16(0x54) == 0) {
            D_8009D2C8->f14 |= bit;
        } else {
            D_800BCD58 |= bit;
        }
        U32(0xF4) |= 0x13;
        op = U32(0x38);
        if (op & 1) {
            unsigned int xq = U16(0x94);
            d = (xq & 0x7F00) >> 8;
            if (!(xq & 0x8000)) {
                m = (d * ((k * 15) >> 8)) >> 7;
            } else {
                m = (d * k) >> 7;
            }
            U16(0x92) = m;
            I32(0x1C) = D_8009C080[U16(0x90)];
            U16(0x8A) = U16(0x88);
            U16(0x8E) = 1;
        }
        if (op & 2) {
            I32(0x20) = D_8009C080[U16(0xA4)];
            U16(0x9E) = U16(0x9C);
            U16(0xA2) = 1;
        }
        if (op & 4) {
            I32(0x24) = D_8009C080[U16(0xB2)];
            U16(0xB0) = 1;
        }
        U16(0xE8) = 0;
        U16(0xEA) = 0;
        I32(0x34) = 0;
    }
    {
        unsigned int xq = U16(0x84);
        U16(0x84) = (xq & 0xFFFD) | ((xq & 1) << 1);
    }
    if (S16(0xE4) != 0) {
        int dv;
        U16(0xE2) += U16(0xE4);
        k = func_8008E840(U16(0x5A), (short)U16(0xE2) + S16(0xDE), S16(0xE0)) << 16;
        dv = U16(0x7E);
        U16(0xE4) = 0;
        U16(0x7A) = dv;
        I32(0x4C) = (int)(k - ((I32(0x30) << 16) + I32(0x34))) / dv;
    }
    U16(0x80) = U16(0xE2);
    U16(0xE6) = U16(0xDE);
}
