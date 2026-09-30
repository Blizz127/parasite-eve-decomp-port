typedef struct {
    int w[2];
} B8;

typedef struct {
    int w[3];
} B12;

typedef struct {
    unsigned char state;
    unsigned char f1;
    unsigned char pad2[2];
    B8 b4;
    int wC;
    int w10;
    B12 b14;
    int w20;
    unsigned short h24;
    unsigned short h26;
    unsigned char b28;
    unsigned char f29;
    unsigned char b2A;
    unsigned char pad2B;
    unsigned short h2C;
    unsigned short h2E;
    unsigned char pad30[0x14];
} CardEntry;

typedef struct {
    unsigned char sel;
    unsigned char f1;
    unsigned char f2;
    unsigned char f3;
    unsigned char f4;
    unsigned char f5;
    unsigned char f6;
    unsigned char f7;
    unsigned char f8;
    unsigned char f9;
    unsigned char count;
    unsigned char fB;
    int fd;
    unsigned char pad10[4];
    short h14;
    short h16;
    unsigned char *p18;
    CardEntry e[15];
} CardRec;

typedef struct {
    B8 b0;
    int w8;
    int wC;
    B12 b10;
    unsigned char pad1C[0xA];
    unsigned short h26;
    unsigned short h28;
    unsigned char b2A;
    unsigned char b2B;
    unsigned char pad2C[0x30];
    unsigned short h5C;
    unsigned short h5E;
    unsigned char pad60[4];
    int w64;
    unsigned char pad68[0x18];
} SaveHdr;

struct DIRENTRY {
    char name[20];
    int attr;
    int size;
    struct DIRENTRY *next;
    int head;
    char system[4];
};

extern CardRec D_800A0ED4[2];
extern SaveHdr D_800A1720[];
extern char D_8009EE70;
extern unsigned char D_8009EED0[];
extern char *D_80092224;
extern char *D_80092230;
extern char D_80010F60[];
extern int D_800A1704;
extern int D_800A1838;
extern int D_800A1854;
extern int D_800A1864;
extern int D_800A186C;

extern void func_80071A84();
extern int func_80071A04(char *, char *, int);
extern int func_80072734(char *, int);
extern void func_80072744(int, int, int);
extern int func_80072754(int, void *, int);
extern int func_80072764(int, void *, int);
extern void func_80072774(int);
extern int func_80072784(char *);
extern int func_80072794(struct DIRENTRY *);
extern int func_800727A4(char *);
extern int func_800727B4(char *, struct DIRENTRY *);
extern void func_80040F80(CardRec *);
extern int func_8004D27C(void);
extern void func_80062CE4(void);
extern unsigned char func_8004D4C4(int, int);
extern void func_8004D298(int);
extern void func_8004CE28(int, int);
extern void func_8004CC50(int, int);
extern void func_80042264(void);
extern void func_8004D9D8(void);

#define SPRINTF_NAME(A2) \
    func_80071A84(&D_8009EE70, D_80092224, (A2), c->e[c->f3].f29 + '0', c->f3 + 'A')

#define RETRY                                   \
    if (c->h16-- > 0) {                         \
        return;                                 \
    }                                           \
    if (!(c->sel & 1)) {                        \
        goto cleanup;                           \
    }                                           \
    func_80040F80(c);                           \
    switch (c->f7) {                            \
    case 1:                                     \
        func_8004CE28(0x3D, 0x3F);              \
        break;                                  \
    case 2:                                     \
        func_8004CC50(0x3E, 0);                 \
        break;                                  \
    }                                           \
    c->f7 = 0;                                  \
    c->f1 = 12;                                 \
    return;

#define MARK                                                    \
    {                                                           \
        CardEntry *e = &c->e[dir.name[19] - 'A'];               \
        e->state = 1;                                           \
        e->f1 = 0;                                              \
        e->f29 = dir.name[18] != '0';                           \
        c->f4 = 1;                                              \
    }

void func_80041108(int idx)
{
    char buf[8];
    struct DIRENTRY dir;
    CardRec *c;
    CardEntry *e;
    SaveHdr *h;
    int i;
    int n;
    int fd;
    int ok;
    short len;
    unsigned char sel;
    int r;
    int up;
    char *nm;
    unsigned char cur;

    c = &D_800A0ED4[idx];
    switch (c->f1) {
    case 1:
        if (c->sel != 1) {
            goto cleanup;
        }
        if (D_800A0ED4[idx].f8 != 4) {
            return;
        }
        if (D_800A0ED4[idx == 0].f8 != 0 && D_800A0ED4[idx == 0].f8 != 4) {
            return;
        }
        D_800A1838 = 1;
        c->f1 = c->fB;
        return;
    case 13:
        if (c->sel == 5) {
            if (D_800A0ED4[idx].f8 != 4) {
                return;
            }
            if (D_800A0ED4[idx == 0].f8 != 0 && D_800A0ED4[idx == 0].f8 != 4) {
                return;
            }
            D_800A1838 = 1;
            c->f1 = 14;
        } else {
            D_800A1864 = -1;
            c->f1 = 0;
        }
        return;
    case 14:
        func_80071A84(buf, D_80010F60, idx);
        D_800A1864 = func_80072784(buf) ? 12 : -1;
        c->sel &= ~4;
        D_800A1838 = 0;
        c->f1 = 0;
        return;
    case 2:
        if (c->sel != 1) {
            goto cleanup;
        }
        D_80092230[2] = idx + '0';
        c->f2 = 0;
        c->f3 = 0;
        c->f6 = 0;
        c->f4 = 0;
        c->f7 = 0;
        c->count = 0;
        for (i = 0; i < 15; i++) {
            c->e[i].state = 2;
        }
        if (func_800727B4(D_80092230, &dir)) {
            if (func_80071A04(dir.name, D_80092224 + 6, 12) == 0) MARK
            for (;;) {
                c->count += dir.size >> 13;
                do {
                } while (0);
                if (func_80072794(&dir) == 0) {
                    break;
                }
                if (func_80071A04(dir.name, D_80092224 + 6, 12) == 0) MARK
            }
            c->h16 = 0;
        } else {
            c->h16--;
        }
        if (c->h16 > 0) {
            return;
        }
        n = c->count;
        for (i = 0; i < 15; i++) {
            if (c->e[i].state == 2) {
                c->e[i].f1 = 1;
            } else {
                n--;
            }
        }
        for (i = 14; i >= 0; i--) {
            if (n == 0) {
                break;
            }
            if (c->e[i].state == 2) {
                c->e[i].state = 3;
                n--;
            }
        }
        c->f2 = 15;
        if (D_800A186C == 0) {
            c->f1 = 15;
            D_800A1838 = 0;
            return;
        }
        {
            CardRec *r = &D_800A0ED4[idx];
            for (i = 0; i < 15; i++) {
                if (r->e[i].state == 1) {
                    break;
                }
            }
            ok = i < 15 || (r->count < 15 && func_8004D27C());
        }
        if (ok) {
            c->f1 = 3;
            c->h16 = 10;
            func_80062CE4();
            c->f5 = func_8004D4C4(c - D_800A0ED4, c->f2);
            return;
        }
        func_80062CE4();
        func_8004D298(idx);
        c->f1 = 12;
        D_800A1838 = 0;
        return;
    case 3:
        if (c->sel != 1) {
            goto cleanup;
        }
        cur = c->f6;
        if (c->f6 < c->f2 * 2) {
            int k;
            n = c->f2 * 2;
            while (1) {
                k = c->f5 + ((cur & 1) * 2 - 1) * ((cur + 1) >> 1);
                if ((unsigned int)k < 15 && c->e[k].f1 == 0) {
                    break;
                }
                cur = c->f6 = cur + 1;
                if (cur >= n) break;
            }
        }
        c->f3 = (c->f6 < c->f2 * 2) ? c->f5 + ((c->f6 & 1) * 2 - 1) * ((c->f6 + 1) >> 1) : 0xFF;
        if (c->f3 != 0xFF) {
            SPRINTF_NAME(D_800A0ED4 < c);
            fd = func_80072734(&D_8009EE70, 1);
            c->fd = fd;
            if (fd >= 0) {
                c->f1 = 8;
                c->p18 = (unsigned char *)&D_800A1720[idx];
                c->h14 = 0x80;
                c->h16 = 30;
                if (c->e[c->f3].state == 1) {
                    func_80072744(c->fd, 0x100, 0);
                }
                return;
            }
            RETRY
        }
        c->f1 = 12;
        D_800A1838 = 0;
        return;
    case 4:
        if (c->sel != 1) {
            goto cleanup;
        }
        c->e[c->f3].f29 = D_800A1704;
        SPRINTF_NAME(D_800A0ED4 < c);
        fd = func_80072734(&D_8009EE70, 0x10200);
        c->fd = fd;
        if (fd >= 0) {
            func_80072774(fd);
            c->fd = -1;
            c->f1 = 6;
            c->h16 = 10;
            return;
        }
        RETRY
    case 5:
        if (c->sel != 1) {
            goto cleanup;
        }
        SPRINTF_NAME(D_800A0ED4 < c);
        fd = func_80072734(&D_8009EE70, 1);
        c->fd = fd;
        if (fd >= 0) {
            c->h16 = 30;
            c->f1 = 7;
            return;
        }
        RETRY
    case 6:
        if (c->sel != 1) {
            goto cleanup;
        }
        SPRINTF_NAME(D_800A0ED4 < c);
        fd = func_80072734(&D_8009EE70, 2);
        c->fd = fd;
        if (fd >= 0) {
            c->f1 = 9;
            c->p18 = D_8009EED0;
            c->h16 = 30;
            return;
        }
        RETRY
    case 8:
        if (c->sel != 1) {
            goto cleanup;
        }
        len = c->h14;
        if (len > 0x80) {
            len = 0x80;
        }
        r = func_80072754(c->fd, c->p18, len);
        if (r > 0) {
            c->p18 += r;
            c->h14 -= r;
            if (c->h14 > 0) {
                return;
            }
            c->f1 = 10;
            return;
        }
        RETRY
    case 7:
        if (c->sel != 1) {
            goto cleanup;
        }
        len = c->h14;
        if (len > 0x400) {
            len = 0x400;
        }
        r = func_80072754(c->fd, c->p18, len);
        if (r > 0) {
            c->p18 += r;
            c->h14 -= r;
            if (c->h14 > 0) {
                return;
            }
            func_80072774(c->fd);
            c->fd = -1;
            c->f1 = 12;
            c->f7 = 0;
            D_800A1854 = 0;
            D_800A1838 = 0;
            func_80042264();
            return;
        }
        RETRY
    case 9:
        if (c->sel != 1) {
            goto cleanup;
        }
        len = c->h14;
        if (len > 0x400) {
            len = 0x400;
        }
        r = func_80072764(c->fd, c->p18, len);
        if (r > 0) {
            c->p18 += r;
            c->h14 -= r;
            if (c->h14 > 0) {
                return;
            }
            func_80072774(c->fd);
            c->fd = -1;
            c->f1 = 3;
            c->f7 = 0;
            D_800A1854 = 0;
            func_8004D9D8();
            func_8004CC50(0x53, 0);
            return;
        }
        RETRY
    case 10:
        if (c->sel != 1) {
            goto cleanup;
        }
        func_80072774(c->fd);
        c->fd = -1;
        e = (CardEntry *)(c->f3 * 68 + (int)c);
        e = (CardEntry *)((char *)e + 0x1C);
        h = &D_800A1720[idx];
        if (e->state == 1) {
            e->h24 = h->h28;
            e->h26 = h->h26;
            e->b28 = h->b2A;
            e->wC = h->w8;
            e->w10 = h->wC;
            e->w20 = h->w64;
            e->b2A = h->b2B;
            e->h2C = h->h5C;
            e->h2E = h->h5E;
            e->b4 = h->b0;
            e->b14 = h->b10;
        }
        e->f1 = 1;
        c->f1 = (++c->f6 < c->f2 * 2) ? 3 : 12;
        if (c->f1 == 12) {
            D_800A1838 = 0;
        }
        return;
    case 11:
        if (c->sel != 1) {
            goto cleanup;
        }
        up = D_800A0ED4 < c;
        SPRINTF_NAME(up);
        nm = &D_8009EE70;
        fd = func_80072734(nm, 1);
        c->fd = fd;
        if (fd >= 0) {
            func_80072774(fd);
            c->fd = -1;
            func_80071A84(nm, D_80092224, up, c->e[c->f3].f29 + '0', c->f3 + 'A');
            if (func_800727A4(nm)) {
                c->f1 = 4;
                c->h16 = 10;
                return;
            }
            RETRY
        }
        RETRY
    case 0:
    case 15:
        return;
    case 12:
        if (c->sel == 1) {
            return;
        }
    cleanup:
        func_80040F80(c);
        return;
    }
}
