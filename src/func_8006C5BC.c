typedef struct {
    unsigned int flags;
    unsigned char pad4[6];
    unsigned char bA;
    unsigned char padB[3];
    unsigned char bE;
    unsigned char padF;
    unsigned char b10;
    unsigned char pad11[3];
    unsigned char f14[0x24];
    unsigned char *f38;
    short h3C;
    short h3E;
    unsigned char pad40[0x5C];
    unsigned char b9C;
    unsigned char b9D;
    unsigned char b9E;
    unsigned char pad9F[0x4F];
    unsigned char state;
    unsigned char padEF[0x2D];
    unsigned char *f11C;
    unsigned char pad120[0x14];
    unsigned char *f134[3];
    unsigned char pad140[0x18];
    unsigned char *f158;
    unsigned char *f15C;
    unsigned char pad160[0x34];
    unsigned char *f194;
    unsigned char pad198[0x28];
    unsigned char *f1C0[1];
} Scene;

typedef struct {
    unsigned char pad0[4];
    unsigned int f4;
    unsigned char b8;
    unsigned char pad9[3];
} Ent;

extern Scene D_800B0CD8;
extern unsigned char D_800B0CE2;
extern signed char D_800B0CE4;
extern int D_800B0DD8;
extern unsigned short D_800930D8[];
extern unsigned short D_800930DA[];
extern unsigned int D_8009D1A0;
extern unsigned char *D_8009D254;
extern unsigned char D_800BEA40[];
extern unsigned char D_800B89F8[];

extern void func_8006CC68(void);
extern int func_8006E6A8(int, unsigned char *, int);
extern int func_8006E7E8(void);
extern void func_8006E1C0(unsigned char *, unsigned char *);
extern int func_8006CDA4(int, int, int, unsigned char *, int, int);
extern void func_8003D050(void *, unsigned char *, unsigned char *, int, int, int, int, int, int *, int);
extern void func_8006698C(void *);
extern void func_8003D834(void *, int, int, void *, void *);

int func_8006C5BC(void)
{
    Scene *s;
    int k;
    int base;
    int idx;
    int r;
    unsigned int i;
    unsigned char *b;
    unsigned char *hdr;
    unsigned char *e;
    unsigned char *t;
    unsigned char *w;
    Ent *en;
    unsigned char **q;
    unsigned char f;
    int buf[16];

    k = D_800B0CE4;
    base = D_800B0DD8;
    s = &D_800B0CD8;
    if ((unsigned int)(D_800B0CE2 - 10) >= 5) {
        return 0;
    }
loop:
    switch (s->state) {
    case 0:
        f = s->bE;
        if (f & 1) {
            s->state = 1;
            goto loop;
        }
        if (f & 2) {
            s->state = 0xB;
            goto loop;
        }
        func_8006CC68();
        return 0;
    case 1:
        if ((unsigned int)(k - 2) < 5) {
            buf[0] = idx = k + 0x26;
            r = func_8006E6A8(base + D_800930D8[idx], s->f194, D_800930DA[idx] - D_800930D8[idx]);
            func_8006CC68();
            if (r != -1) {
                s->state = 2;
            }
            return 1;
        }
        s->state = 4;
        goto loop;
    case 2:
        r = func_8006E7E8();
        if (r == -1) {
            s->state = 1;
            return 1;
        }
        if (r != 0) {
            if (!(D_8009D1A0 & 2)) {
                func_8006CC68();
            }
            return 1;
        }
        s->state = 3;
        goto loop;
    case 3:
        b = s->f194;
        hdr = b + *(int *)(b + 4);
        e = b + (*(unsigned int *)(hdr + 0x28) & 0x3FFFFF);
        i = 0;
        if (*(unsigned int *)(hdr + 0x28) >> 22) {
            do {
                func_8006E1C0(e + i * 0x14, b);
            } while (++i < *(unsigned int *)(hdr + 0x28) >> 22);
        }
        s->state = 4;
        goto loop;
    case 4:
        buf[0] = idx = ((s->bA - 10) / 2) * 8 + k + 0x16;
        if (func_8006E6A8(base + D_800930D8[idx], s->f158, D_800930DA[idx] - D_800930D8[idx]) != -1) {
            s->state = 5;
        }
        if (!(D_8009D1A0 & 2)) {
            func_8006CC68();
        }
        return 1;
    case 5:
        r = func_8006E7E8();
        if (r == -1) {
            s->state = 4;
            return 1;
        }
        if (r != 0) {
            if (!(D_8009D1A0 & 2)) {
                func_8006CC68();
            }
            return 1;
        }
        s->state = 6;
        goto loop;
    case 6:
        b = s->f158;
        hdr = b + *(int *)(b + 4);
        en = (Ent *)(b + (*(unsigned int *)(hdr + 0x10) & 0x3FFFFF));
        i = 0;
        if (*(unsigned int *)(hdr + 0x10) >> 22) {
            do {
                s->f1C0[((unsigned char *)&en[i].f4)[3]] = b + (en[i].f4 & 0xFFFFFF);
            } while (++i < *(unsigned int *)(hdr + 0x10) >> 22);
        }
        s->b10 = 0;
        for (i = 0; i < 3; i++) {
            s->f134[i] = 0;
        }
        if (*(unsigned int *)(hdr + 0x2C) & 0xFFC00000) {
            Ent *tn;

            tn = (Ent *)(b + (*(unsigned int *)(hdr + 0x2C) & 0x3FFFFF));
            i = 0;
            if (*(unsigned int *)(hdr + 0x2C) >> 22) {
                do {
                    s->f134[i] = b + (tn[i].f4 & 0xFFFFFF);
                } while (++i < *(unsigned int *)(hdr + 0x2C) >> 22);
            }
            s->b10 = tn->b8;
        }
        if (D_8009D1A0 & 2) {
            s->state = 7;
            return 1;
        }
        if (s->bE & 2) {
            s->state = 0xB;
            goto loop;
        }
        func_8006CC68();
        s->state = 0;
        s->bE &= 0xFC;
        return 0;
    case 7:
        if (func_8006CDA4(1, s->b10, 0, s->f194, 0x21, 0) == 0) {
            s->state = 0xB;
            goto loop;
        }
        return 1;
    case 0xB:
        s->state = 0xC;
        s->bE |= 1;
        return 1;
    case 0xC:
        s->state = 0xD;
        return 1;
    case 0xD:
        b = s->f158;
        hdr = b + *(int *)(b + 4);
        en = (Ent *)(b + (*(unsigned int *)(hdr + 0x10) & 0x3FFFFF));
        i = 0;
        if (*(unsigned int *)(hdr + 0x10) >> 22) {
            do {
                s->f1C0[((unsigned char *)&en[i].f4)[3]] = b + (en[i].f4 & 0xFFFFFF);
            } while (++i < *(unsigned int *)(hdr + 0x10) >> 22);
        }
        s->b10 = 0;
        for (i = 0; i < 3; i++) {
            s->f134[i] = 0;
        }
        if (*(unsigned int *)(hdr + 0x2C) & 0xFFC00000) {
            Ent *tn;

            tn = (Ent *)(b + (*(unsigned int *)(hdr + 0x2C) & 0x3FFFFF));
            i = 0;
            if (*(unsigned int *)(hdr + 0x2C) >> 22) {
                do {
                    s->f134[i] = b + (tn[i].f4 & 0xFFFFFF);
                } while (++i < *(unsigned int *)(hdr + 0x2C) >> 22);
            }
            s->b10 = tn->b8;
        }
        if (D_8009D254 == 0) {
            {
                int z = s->flags & 2;
                return z == 0;
            }
        }
        if (D_8009D1A0 & 2) {
            t = b + (((Ent *)(b + (*(unsigned int *)(hdr + 0xC) & 0x3FFFFF)))->f4 & 0xFFFFFF);
        } else {
            t = s->f11C;
        }
        func_8003D050(s->f14, t, s->f15C, 0x2C0, 0x80, 0, 0x1C2, 0, buf, 1);
        if (s->f38 == 0 && D_8009D254 != 0) {
            s->f38 = D_8009D254 + 0x1B4;
            s->h3C = 3;
            s->h3E = 0x12;
        }
        s->b9C = 0x80;
        s->b9D = 0xC;
        s->b9E = 0x18;
        func_8006698C(s->f14);
        func_8003D834(s->f14, 0, 0, D_800BEA40, D_800B89F8);
        s->state = 0;
        s->bE &= 0xFC;
        return 1;
    }
    return 0;
}
