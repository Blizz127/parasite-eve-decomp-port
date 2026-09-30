typedef struct {
    unsigned off : 22;
    unsigned n : 10;
} Span;

typedef struct {
    unsigned int f0;
    unsigned int f4;
    unsigned short h8;
    unsigned short hA;
} Ent;

typedef struct {
    unsigned int flags;
    unsigned char pad4[0xD6];
    signed char bDA[2];
    signed char bDC[4];
    unsigned char padE0[0x11];
    unsigned char state;
    unsigned char padF2[0x36];
    int f128[2];
    unsigned char pad130[0x5C];
    unsigned char *f18C;
    unsigned char pad190[4];
    unsigned char *f194;
} Scene;

extern Scene D_800B0CD8;
extern unsigned char *D_800B0E64;
extern Ent *D_8009D180;
extern short D_8009D184;

extern void func_80086FF8(void);
extern int func_8006CDA4(int, int, int, unsigned char *, int, int);
extern void func_80071A34(int, unsigned char *, unsigned int);

int func_8006D2B8(int id, int check, int slot, int *out, int arg5)
{
    Scene *s;
    unsigned char *hdr;
    register int done asm("$17");
    int ret;
    int sel;
    int i;
    int n;
    Ent *p;
    unsigned char c;

    ret = 0;
    done = 0;
    s = &D_800B0CD8;
    sel = slot != 0;
    hdr = D_800B0E64 + *(int *)(D_800B0E64 + 4);
    do {
        switch (s->state) {
        case 0:
            D_8009D184 = -1;
            p = (Ent *)(s->f18C + ((Span *)(hdr + 0x30))->off);
            D_8009D180 = p;
            for (i = 0; i < ((Span *)(hdr + 0x30))->n; i++, p++) {
                if (p->hA == id) {
                    D_8009D180 = p;
                    D_8009D184 = p->h8;
                    break;
                }
            }
            if (D_8009D184 == -1) {
                for (i = 0; i < 2; i++) {
                    if (s->bDC[i * 2] == id) {
                        c = ((unsigned char *)s + i)[0xDA];
                        D_8009D180 = 0;
                        D_8009D184 = (signed char)c;
                        break;
                    }
                }
            }
            if (D_8009D184 == -1) {
                *out = -1;
            } else if (check != 0) {
                if (s->bDC[0] == id) {
                    if (s->flags & 0x40) {
neg:
                        *out = -2;
                    } else {
                        *out = 0;
                    }
                } else if (s->bDC[2] == id) {
                    if (s->flags & 0x80) {
                        goto neg;
                    }
                    *out = 1;
                } else {
                    if (slot == 0) {
                        func_80086FF8();
                    }
                    s->state = 7;
                    break;
                }
            } else {
                if (s->bDC[0] == id) {
                    s->bDC[0] = -1;
                    s->bDA[0] = -1;
                    s->flags &= ~0x40;
                    *out = 0;
                }
                if (s->bDC[2] == id) {
                    s->bDC[2] = -1;
                    s->bDA[1] = -1;
                    s->flags &= ~0x80;
                    *out = 1;
                }
            }
            ret = 0;
            done = 1;
            break;
        case 7:
            if (func_8006CDA4(0, D_8009D184, 0, s->f194, 0x21, arg5) == 1) {
                ret = 1;
                done = (arg5 ^ 1) & 1;
            } else {
                s->state = 9;
            }
            break;
        case 9:
            *out = sel;
            if (D_8009D180 != 0) {
                func_80071A34(s->f128[sel], s->f18C + (D_8009D180->f4 & 0xFFFFFF), D_8009D180->f0 & 0xFFFFFF);
            }
            ret = 0;
            done = 1;
            s->bDC[*out << 1] = id;
            s->bDA[*out] = D_8009D184;
            s->state = 0;
            break;
        }
    } while (!done);
    return ret;
}
