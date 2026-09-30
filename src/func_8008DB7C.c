typedef struct {
    unsigned char pad0[0x2C];
    unsigned int f2C;
    unsigned char pad30[0x20];
    int f50;
    unsigned char pad54[2];
    unsigned short h56;
    unsigned short h58;
    unsigned char pad5A[0xC2];
} Voice;

typedef struct {
    unsigned int f0;
    unsigned int f4;
    unsigned char pad8[0xC];
    unsigned int f14;
    unsigned int f18;
    unsigned int f1C;
    int f20;
    int f24;
    unsigned int f28;
    unsigned char pad2C[0x14];
    int f40;
    int f44;
    unsigned char pad48[0xA];
    unsigned short h52;
    unsigned short h54;
    unsigned char pad56[2];
    unsigned short h58;
    unsigned char pad5A[2];
    unsigned short h5C;
    unsigned short h5E;
    unsigned short h60;
    unsigned short h62;
    unsigned short h64;
    unsigned char pad66[2];
} Ctl;

#define TEMPO(c) (*(unsigned short *)((unsigned char *)(c) + 0x22))

extern Ctl *D_8009D2C8;
extern unsigned char D_8009D2D2;
extern unsigned int D_8009D2DC;
extern unsigned int D_8009D2C4;
extern int D_8009D22C;
extern int D_8009D268;
extern Voice D_800B8AC0[];
extern Voice D_800BA560[];
extern Voice D_800BC000[];
extern unsigned int D_800BCD50;
extern unsigned short D_800BCD66;
extern unsigned int D_800BCD68;
extern unsigned int D_800BCD5C;
extern unsigned int D_800BCD58;

extern void func_80089328(void);
extern void func_8008E8D0(Voice *, unsigned int);
extern void func_80087AA8(Voice *, unsigned int);
extern void func_80087FA0(Voice *, unsigned int);
extern void func_8008D820(void *, void *, int);
extern void func_8008CA84(void);
extern void func_8008D844(void);
extern void func_80089784(void);

void func_8008DB7C(void)
{
    unsigned int t;
    unsigned int m;
    unsigned int v;
    unsigned int bit;
    unsigned int act;
    Voice *p;
    Ctl *c;
    Ctl *d;
    Ctl *e;
    unsigned int w;

    func_80089328();
    if (D_8009D2C8->f4 != 0) {
        t = TEMPO(D_8009D2C8);
        m = D_8009D2D2;
        if (m != 0) {
            if (m < 0x80) {
                t += (t * m) >> 7;
            } else {
                t = (t * m) >> 8;
            }
        }
        c = D_8009D2C8;
        v = c->f28 + t;
        c->f28 = v;
        if ((v & 0xFFFF0000) || (D_8009D2DC & 4)) {
            c->f28 = v & 0xFFFF;
            do {
                bit = 1;
                p = D_800B8AC0;
                act = D_8009D2C8->f4;
                do {
                    if (act & bit) {
                        p->h56--;
                        p->h58--;
                        if (p->h56 == 0) {
                            func_8008E8D0(p, bit);
                        } else if (p->h58 == 0) {
                            D_8009D2C8->f18 |= bit;
                            D_8009D2C8->f14 &= ~bit;
                        }
                        func_80087AA8(p, bit);
                        act ^= bit;
                    }
                    p++;
                    bit <<= 1;
                } while (act != 0);
                c = D_8009D2C8;
                if (c->h52 != 0) {
                    c->h52--;
                    c->f20 += c->f24;
                }
                c = D_8009D2C8;
                if (c->h58 != 0) {
                    *(unsigned short *)((unsigned char *)c + 0x58) -= 1;
                    *(int *)((unsigned char *)c + 0x40) += *(int *)((unsigned char *)c + 0x44);
                    D_8009D2C4 |= 0x80;
                }
                d = D_8009D2C8;
                if (d->h60 != 0) {
                    if (++d->h62 == d->h60) {
                        d->h62 = 0;
                        if (++d->h5E == d->h5C) {
                            d->h5E = 0;
                            d->h64++;
                            if (D_8009D22C == 0) {
                                break;
                            }
                            D_8009D22C--;
                        }
                    }
                }
            } while (D_8009D22C != 0);
        }
    }
    c = D_8009D2C8;
    if (c[1].f4 != 0) {
        t = TEMPO(c + 1);
        D_8009D2C8 = c + 1;
        m = D_8009D2D2;
        if (m != 0) {
            if (m < 0x80) {
                t += (t * m) >> 7;
            } else {
                t = (t * m) >> 8;
            }
        }
        e = D_8009D2C8;
        w = e->f28 + t;
        e->f28 = w;
        if ((w & 0xFFFF0000) || (D_8009D2DC & 4)) {
            bit = 1;
            p = D_800BA560;
            act = e->f4;
            e->f28 = w & 0xFFFF;
            do {
                if (act & bit) {
                    p->h56--;
                    p->h58--;
                    if (p->h56 == 0) {
                        func_8008E8D0(p, bit);
                    } else if (p->h58 == 0) {
                        D_8009D2C8->f18 |= bit;
                        D_8009D2C8->f14 &= ~bit;
                    }
                    func_80087AA8(p, bit);
                    act ^= bit;
                }
                p++;
                bit <<= 1;
            } while (act != 0);
            c = D_8009D2C8;
            if (c->h52 != 0) {
                c->h52--;
                c->f20 += c->f24;
            }
            c = D_8009D2C8;
            if (c->h58 != 0) {
                c->h58--;
                c->f40 += c->f44;
            }
            d = D_8009D2C8;
            if (d->h60 != 0) {
                if (++d->h62 == d->h60) {
                    d->h62 = 0;
                    if (++d->h5E == d->h5C) {
                        d->h5E = 0;
                        d->h64++;
                        if (D_8009D22C != 0) {
                            D_8009D22C--;
                        }
                    }
                }
            }
        }
        D_8009D2C8 = D_8009D2C8 - 1;
    }
    c = D_8009D2C8;
    if (c->f4 == 0 && c->f1C == 0 && c[1].f4 != 0) {
        func_8008D820(c + 1, c, sizeof(Ctl));
        func_8008D820(D_800BA560, D_800BA560 - 24, 0x1AA0);
        D_8009D2C8[1].h54 = 0;
        D_8009D2C8[1].f4 = 0;
    }
    act = D_800BCD50;
    if (act != 0) {
        v = D_800BCD68 + D_800BCD66;
        D_800BCD68 = v;
        if ((v & 0xFFFF0000) || (D_8009D2DC & 4)) {
            D_800BCD68 = v & 0xFFFF;
            bit = 0x1000;
            p = D_800BC000;
            do {
                if (act & bit) {
                    if (!(D_8009D2DC & 2) || (p->f2C & 0x2000000)) {
                        p->h56--;
                        p->f50++;
                        p->h58--;
                        if (p->h56 == 0) {
                            func_8008E8D0(p, bit);
                        } else if (p->h58 == 0) {
                            D_800BCD5C |= bit;
                            D_800BCD58 &= ~bit;
                        }
                        func_80087FA0(p, bit);
                    }
                    act ^= bit;
                }
                p++;
                bit <<= 1;
            } while (act != 0);
        }
    }
    if (D_8009D268 == 0) {
        func_8008CA84();
    }
    func_8008D844();
    func_80089784();
}
