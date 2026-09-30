typedef struct {
    unsigned char f0;
    unsigned char f1;
    unsigned char f2;
    unsigned char f3;
    int f4;
    int f8;
} Slot;

typedef struct {
    Slot s;
    unsigned char pad[0xA0C - 12];
} BigSlot;

typedef struct {
    Slot s;
    unsigned char pad[0x10C - 12];
} SmallSlot;

typedef struct {
    unsigned off : 22;
    unsigned n : 10;
} Span;

typedef struct {
    unsigned char pad0[4];
    unsigned off : 24;
    unsigned id : 8;
    unsigned w : 10;
    unsigned x : 11;
    unsigned y : 11;
} Ent;

typedef struct {
    unsigned int flags;
    unsigned char pad4[0xEB];
    unsigned char state;
    unsigned char padF0[0x10];
    unsigned char *f100;
    unsigned char pad104[0x84];
    unsigned char *f188;
    unsigned char *f18C;
    unsigned char pad190[4];
    unsigned char *f194;
} Scene;

typedef void (*Handler)(void);

extern Scene D_800B0CD8;
extern unsigned int D_8009D1A0;
extern BigSlot *D_800942E4;
extern SmallSlot *D_800942E8;
extern Handler **D_800942E0;
extern unsigned short D_800930E2;
extern unsigned short D_800930E4;

extern int func_8006E6A8(unsigned char *, unsigned char *, int);
extern int func_8006E7E8(void);
extern void func_8006E1C0(unsigned char *, unsigned char *);
extern Ent *func_8006E498(unsigned char *, int);
extern void func_8007506C(short *, void *);

int func_8006914C(int arg)
{
    Scene *s;
    int i;
    unsigned char *base;
    unsigned char *hdr;
    unsigned char *e;
    Handler *h;
    Slot *p;
    unsigned char *b0, *h0, *e0;
    BigSlot **pb;
    SmallSlot **ps;
    int r;
    Ent *en;
    int key;
    short rect[4];

    s = &D_800B0CD8;
loop:
    switch (s->state) {
    case 0:
        if (!(D_8009D1A0 & 0x80)) {
            pb = &D_800942E4;
            *pb = (BigSlot *)s->f188;
            for (i = 0; i < 11; i++) {
                BigSlot **q = &D_800942E4;
                p = &(*q)[i].s;
                p->f0 = 0;
                p->f8 = 0;
                p->f1 = 0xFF;
                p->f2 = 0xFF;
                p->f3 = 0xFF;
                p->f4 = 0;
            }
            ps = &D_800942E8;
            *ps = (SmallSlot *)(s->f188 + 0x6E84);
            for (i = 0; i < 11; i++) {
                SmallSlot **q = &D_800942E8;
                p = &(*q)[i].s;
                p->f0 = 0;
                p->f8 = 0;
                p->f1 = 0xFF;
                p->f2 = 0xFF;
                p->f3 = 0xFF;
                p->f4 = 0;
            }
            for (i = 0; i < 8; i++) {
                h = D_800942E0[i];
                if (h != 0 && *h != 0) {
                    (*h)();
                }
            }
            h = D_800942E0[0x55];
            if (h != 0 && *h != 0) {
                (*h)();
            }
            b0 = s->f18C;
            h0 = b0 + *(int *)(b0 + 4);
            e0 = b0 + ((Span *)(h0 + 4))->off;
            for (i = 0; i < ((Span *)(h0 + 4))->n; i++) {
                if ((unsigned int)(e0[i * 12 + 7] - 8) < 0x4D) {
                    h = D_800942E0[e0[i * 12 + 7]];
                    if (h != 0 && *h != 0) {
                        (*h)();
                    }
                }
            }
            s->flags = (s->flags & ~0x10000) | 8;
            D_8009D1A0 |= 0x80;
        }
        if ((arg != 0 || (D_8009D1A0 & 2)) && (s->flags & 8)) {
            s->state = 0x34;
            goto loop;
        }
        s->state = 0;
        return 0;
    case 0x34:
        if (func_8006E6A8(s->f100 + D_800930E2, s->f194, D_800930E4 - D_800930E2) != -1) {
            s->state = 0x35;
        }
        return 1;
    case 0x35:
        r = func_8006E7E8();
        if (r == -1) {
            s->state = 0x34;
            return 1;
        }
        if (r != 0) {
            return 1;
        }
        s->state = 0x36;
        goto loop;
    case 0x36:
        base = s->f194;
        hdr = base + *(int *)(base + 4);
        e = base + ((Span *)(hdr + 0x28))->off;
        for (i = 0; i < ((Span *)(hdr + 0x28))->n; i++) {
            func_8006E1C0(e + i * 0x14, base);
        }
        base = s->f18C;
        key = 0x73DECD80;
        while ((en = func_8006E498(base, key)) != 0) {
            rect[0] = en->x;
            rect[1] = en->y;
            rect[2] = en->w;
            rect[3] = en->id ? en->id : 0x100;
            func_8007506C(rect, (unsigned char *)en + en->off);
            key += 4;
        }
        s->state = 0;
        s->flags &= ~8;
        return 0;
    }
    return 0;
}
