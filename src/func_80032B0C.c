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

typedef struct Dmg {
    short val;
    unsigned short x;
    unsigned short y;
    unsigned char c;
    unsigned char kind;
} Dmg;

extern int D_8009CDDC;
extern int D_8009D230;
extern Ent D_800B01C0[][10][5];
extern unsigned char *D_800B0E38[];
extern void func_80077AC4(unsigned char *, Ent *);

#define E2 (*(Ent (*)[10][5])&D_800B01C0[D_8009CDDC])[D_8009D230]

void func_80032B0C(unsigned char mode, Dmg *d)
{
    char buf[8];
    short val;
    short q;
    signed char n;
    short x;
    int y;

    val = d->val;
    if (val < 0) {
        if (mode == 1) {
            x = d->x - 0x10;
            { int t = d->c - 0x1E; y = d->y + t; }
            (&E2[0].sp)->r0 = d->c * 4;
            (&E2[0].sp)->g0 = 0;
            (&E2[0].sp)->b0 = 0;
            (&E2[0].sp)->u0 = 0x50;
            (&E2[0].sp)->v0 = 0xE0;
            (&E2[0].sp)->w = 0x18;
            (&E2[0].sp)->x0 = x;
            (&E2[0].sp)->y0 = y;
            (&E2[0].sp)->h = 8;
            func_80077AC4(D_800B0E38[D_8009CDDC] + 0x1C, &E2[0]);
            goto end;
        }
        val = 0;
    }
    n = 0;
    while (q = val / 10, buf[n] = val - q * 10, (val = q) != 0) {
        n++;
    }
    x = d->x - (n + 1) * 4;
    { int t = d->c - 0x1E; y = d->y + t; }
    for (; n >= 0; n--) {
        switch (d->kind) {
        case 0:
            (&E2[n].sp)->r0 = d->c * 4;
            (&E2[n].sp)->g0 = d->c * 4;
            (&E2[n].sp)->b0 = d->c * 4;
            break;
        case 1:
            (&E2[n].sp)->r0 = 0;
            (&E2[n].sp)->g0 = d->c * 4;
            (&E2[n].sp)->b0 = 0;
            break;
        case 2:
            (&E2[n].sp)->r0 = d->c * 4;
            (&E2[n].sp)->g0 = d->c * 4;
            (&E2[n].sp)->b0 = 0;
            break;
        case 3:
            (&E2[n].sp)->r0 = d->c * 4;
            (&E2[n].sp)->g0 = 0;
            (&E2[n].sp)->b0 = d->c * 4;
            break;
        }
        x += 8;
        (&E2[n].sp)->u0 = buf[n] * 8;
        (&E2[n].sp)->v0 = 0xE0;
        (&E2[n].sp)->x0 = x;
        (&E2[n].sp)->y0 = y;
        (&E2[n].sp)->w = 8;
        (&E2[n].sp)->h = 8;
        func_80077AC4(D_800B0E38[D_8009CDDC] + 0x1C, &E2[n]);
    }
end:
    D_8009D230++;
}
