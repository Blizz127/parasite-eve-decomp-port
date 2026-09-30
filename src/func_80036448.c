typedef struct Contact {
    unsigned char pad0[8];
    unsigned short f8;
    unsigned char padA[2];
    int fC;
    unsigned char pad10[8];
    int f18;
    int f1C;
    unsigned char pad20[4];
    struct Contact *f24;
    struct Contact *f28;
} Contact;

typedef struct Hit {
    short f0;
    short f2;
    short f4;
    short f6;
    short f8;
    unsigned char padA[2];
} Hit;

typedef struct Def {
    unsigned char pad0[3];
    unsigned char f3;
} Def;

typedef struct Ent {
    unsigned char pad0[4];
    struct Ent *next;
    unsigned char pad8[4];
    unsigned char fC;
    unsigned char fD;
    unsigned char padE[0x24 - 0xE];
    unsigned short f24;
    unsigned short f26;
    int f28;
    unsigned char pad2C[4];
    int f30;
    unsigned char pad34[0x3A - 0x34];
    unsigned short f3A;
    unsigned char pad3C[4];
    int f40;
    unsigned char pad44[4];
    int f48;
    unsigned char pad4C[0x52 - 0x4C];
    unsigned short f52;
    unsigned char pad54[0x98 - 0x54];
    unsigned int f98;
    unsigned char pad9C[8];
    Contact *fA4;
    unsigned char padA8[0x18C - 0xA8];
    struct Ent *f18C;
    unsigned char pad190[4];
    void (*f194)(struct Ent *, int, struct Ent *, int);
    unsigned char pad198[8];
    int f1A0;
    unsigned char pad1A4[8];
    int f1AC;
    unsigned char pad1B0[4];
    Def *f1B4;
    unsigned char pad1B8[0x21C - 0x1B8];
    short f21C;
    short f21E;
    short f220;
    unsigned char pad222[2];
    short f224;
    unsigned char pad226[2];
    short f228;
    short f22A;
    short f22C;
    unsigned char pad22E[2];
    short f230;
    unsigned char pad232[2];
    Hit *f234;
} Ent;

extern Ent *D_8009D20C;
extern Ent *D_8009D254;
extern unsigned int D_8009D1A0;

extern Contact *func_80012700(int, int);
extern void func_80035F54(Ent *);

#define LINK_CONTACT(x, y, n)                                                     \
    if ((x)->f1A0) {                                                           \
        found = 0;                                                             \
        c = (x)->fA4;                                                          \
    top##n:                                                                    \
        if (c == 0) {                                                          \
            goto out##n;                                                       \
        }                                                                      \
        if ((c->f8 & 1) && !(c->f8 & 0x10) && c->fC == (y)->f24) {              \
            found = 1;                                                         \
        }                                                                      \
        c = c->f24;                                                            \
        if (!found) {                                                          \
            goto top##n;                                                       \
        }                                                                      \
    out##n:                                                                    \
        if (!found) {                                                          \
            c = func_80012700((x)->f1A0, 0);                                   \
            c->f8 |= 1;                                                        \
            c->fC = (y)->f24;                                                  \
            c->f18 = (y)->fC;                                                  \
            c->f1C = (y)->fD;                                                  \
            t = (x)->fA4;                                                      \
            if (t) {                                                           \
                c->f24 = t;                                                    \
                t->f28 = c;                                                    \
            }                                                                  \
            (x)->fA4 = c;                                                      \
        }                                                                      \
    }

void func_80036448(void) {
    Ent *a;
    Ent *b;
    Contact *c;
    Contact *t;
    int found;
    int w[4];
    short *pa;
    short *pb;
    int ra;
    int rb;
    int rbb;
    int d;
    int r2;
    Hit *ha;
    Hit *hb;
    short *qa;
    short *qb;
    unsigned int i;
    unsigned int j;
    int r1;

    for (a = D_8009D20C; a; a = a->next) {
        a->f98 &= ~0x2040000;
    }
    for (a = D_8009D20C; a; a = a->next) {
        if (a->f98 & 0x20) {
            continue;
        }
        pa = &a->f228;
        pb = &a->f21C;
        ra = (a->f224 * a->f26) / 4096;
        rb = (a->f230 * a->f26) / 4096;
        for (b = a->next; b; b = b->next) {
            if (a->f18C == b) {
                continue;
            }
            if (b->f18C == a) {
                continue;
            }
            if ((a->f98 & 0x20000) && b != D_8009D254) {
                continue;
            }
            if ((b->f98 & 0x20000) && a != D_8009D254) {
                continue;
            }
            if (b->f98 & 0x20) {
                continue;
            }
            r2 = (b->f230 * b->f26) / 4096;
            {
                int x0 = pa[0] - b->f228;
                int x1;
                int x2;
                x0 = x0 * x0;
                x1 = pa[1] - b->f22A;
                x1 = x1 * x1;
                x2 = pa[2] - b->f22C;
                x2 = x2 * x2;
                d = x0 + x1 + x2;
            }
            r2 += rb;
            r2 *= r2;
            if (!a->f1AC || !b->f1AC) {
                if (r2 < d) {
                    continue;
                }
            link:
                LINK_CONTACT(a, b, 1)
                LINK_CONTACT(b, a, 2)
                continue;
            }
            if (r2 < d) {
                continue;
            }
            if (pb[4] == 0 || b->f224 == 0) {
                goto hit;
            }
            {
                int d2;

                r2 = (b->f224 * b->f26) / 4096;
                {
                    int y0 = pb[0] - b->f21C;
                    int y1;
                    y0 = y0 * y0;
                    y1 = pb[2] - b->f220;
                    y1 = y1 * y1;
                    d2 = y0 + y1;
                }
                r2 += ra;
                r2 *= r2;
                if (!(r2 < d2)) {
                    w[0] = (a->f28 - a->f40) >> 16;
                    w[1] = (a->f30 - a->f48) >> 16;
                    w[2] = pb[0] - b->f21C;
                    w[3] = pb[2] - b->f220;
                    d = w[0] * w[2];
                    d += w[1] * w[3];
                    if (d < 0) {
                        func_80035F54(a);
                    }
                    w[0] = (b->f28 - b->f40) >> 16;
                    w[1] = (b->f30 - b->f48) >> 16;
                    d = w[0] * w[2];
                    d += w[1] * w[3];
                    if (d > 0) {
                        func_80035F54(b);
                    }
                    if (a != D_8009D254 && !(a->f98 & 0x1000000)) {
                        a->f3A = a->f52;
                    }
                    if (b != D_8009D254 && !(b->f98 & 0x1000000)) {
                        b->f3A = b->f52;
                    }
                    if (a == D_8009D254) {
                        b->f98 |= 0x2000000;
                    } else if (b == D_8009D254) {
                        a->f98 |= 0x2000000;
                    }
                    LINK_CONTACT(a, b, 3)
                    LINK_CONTACT(b, a, 4)
                    if (!(D_8009D1A0 & 2)) {
                        continue;
                    }
                } else {
                    if (!(D_8009D1A0 & 2) && !(r2 + 10000 < d2)) {
                        goto link;
                    }
                }
            }
        hit:
            if (a->fC && b->fC) {
                a->f98 |= 0x40000;
                b->f98 |= 0x40000;
                continue;
            }
            ha = a->f234;
            for (i = 0; i < a->f1B4->f3; i++, ha++) {
                r1 = (ha->f8 * a->f26) / 4096;
                hb = b->f234;
                for (j = 0; j < b->f1B4->f3; j++, hb++) {
                    r2 = (hb->f8 * b->f26) / 4096;
                    {
                        int z0 = ha->f0 - hb->f0;
                        int z1;
                        int z2;
                        z0 = z0 * z0;
                        z1 = ha->f2 - hb->f2;
                        z1 = z1 * z1;
                        z2 = ha->f4 - hb->f4;
                        z2 = z2 * z2;
                        d = z0 + z1 + z2;
                    }
                    r2 += r1;
                    r2 *= r2;
                    if (!(r2 < d)) {
                        if (a->f194) {
                            a->f194(a, ha->f6, b, hb->f6);
                        }
                        if (b->f194) {
                            b->f194(b, hb->f6, a, ha->f6);
                        }
                    }
                }
            }
        }
    }
}
