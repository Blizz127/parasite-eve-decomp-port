typedef struct {
    unsigned char pad0[3];
    unsigned char code;
    unsigned char r, g, b;
    unsigned char flag;
    unsigned short x, y;
    unsigned char u, v;
    unsigned short clut;
} Spr;

typedef struct {
    unsigned char pad0[3];
    unsigned char code;
    unsigned int w;
} Dm;

typedef struct {
    unsigned int w0;
    unsigned int w1;
} Rec;

extern unsigned char D_800BD024;
extern unsigned char *volatile D_800B1624;
extern int func_80077AA4(int, int);
extern int func_80077A64(int, int, int, int);

int func_80066F60(unsigned char *a0, unsigned char *a1, unsigned char **a2)
{
    unsigned int *pos;
    Rec *rec;
    unsigned int k;
    unsigned int n;
    unsigned char *fp;
    unsigned int i;
    int c80;
    int mx;
    int c;
    unsigned int w;
    unsigned int w1;
    unsigned int v;

    c = D_800BD024;
    k = 0;
    pos = (unsigned int *)(a0 + *(int *)(a0 + 0x28));
    n = *(unsigned short *)(a0 + 0x26);
    c80 = 0x80;
    *(unsigned char **)(a0 + 0x30) = a1;
    fp = a1 + n * 32;
    *(unsigned char **)(a0 + 0x34) = fp;
    rec = (Rec *)(a0 + *(int *)(a0 + 0x2C));
    *(unsigned short *)(a0 + 0x18) = *(unsigned short *)(a0 + 0xC) + *(unsigned short *)(D_800B1624 + 0x38);
    *(unsigned short *)(a0 + 0x1A) = *(unsigned short *)(a0 + 0xE) + *(unsigned short *)(D_800B1624 + 0x3A);
    mx = c + 0x1DF;
    do {
        Spr *sp;
        Dm *dm;
        i = 0;
        if (i < n) {
        sp = (Spr *)a1;
        dm = (Dm *)fp;
        for (; i < n; i++) {
            Spr *s = &sp[i];
            asm("" : "=r"(dm) : "0"(dm));
            s->code = 3;
            s->flag = 0x7C;
            s->r = c80;
            s->g = c80;
            s->b = c80;
            s->x = *(unsigned short *)(a0 + 0x18) + (pos[i] >> 22);
            s->y = *(unsigned short *)(a0 + 0x1A) + ((pos[i] >> 12) & 0x3FF);
            s->u = ((unsigned char *)&rec[i])[4];
            s->v = ((unsigned char *)&rec[i])[3];
            if (rec[i].w1 & 0x10000000) {
                s->flag |= 2;
            } else {
                s->flag &= ~2;
            }
            w1 = rec[i].w0;
            s->clut = func_80077AA4((w1 >> 5) & 0x3F0, w1 & 0x1FF);
            v = rec[i].w0 & 0x1FF;
            if (mx < (int)v) {
                mx = v;
            }
            dm->code = 1;
            w = rec[i].w0;
            dm->w = (func_80077A64(1, (w >> 22) & 3, (w >> 10) & 0x3C0, (w >> 7) & 0x100) & 0x9FF) | 0xE1000600;
            dm++;
            asm("" : : "r"(a0), "r"(a0), "r"(a0), "r"(a0));
        }
        }
        a1 += n * 16;
        fp += n * 8;
    } while (++k < 2);
    asm("" : : "r"(mx));
    *a2 = fp;
    D_800BD024 = mx + 0x21;
    return 0;
}
