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

typedef struct Rec {
    unsigned char pad0[0x4C];
    unsigned int f4C;
} Rec;

extern Rec *D_8009D278;
extern unsigned char D_8009CE80;
extern int D_8009CDDC;
extern Ent D_8009E960[][13];
extern unsigned char *D_800B0E38[];
extern void func_80077AC4(unsigned char *, Ent *);

void func_800334AC(void)
{
    Rec *r;
    unsigned int *fl;
    short x;
    short y;
    short dx;
    short k;

    r = D_8009D278;
    fl = &r->f4C;
    x = 0xF;
    if ((unsigned char)(D_8009CE80 - 1) < 2) {
        x = 0x121;
    }
    y = 0xF;
    if (D_8009CE80 < 2) {
        y = 0xC1;
    }
    dx = 0x10;
    if (x == 0x121) {
        dx = -0x10;
    }
    if (r->f4C & 0xC) {
        k = (r->f4C & 0xC) ^ 0xC;
        k = k == 0;
        k <<= 2;
        (&(D_8009E960[D_8009CDDC] + k)->sp)->x0 = x;
        (&(D_8009E960[D_8009CDDC] + k)->sp)->y0 = y;
        func_80077AC4(D_800B0E38[D_8009CDDC] + 0x1C, (D_8009E960[D_8009CDDC] + k));
        x += dx;
    }
    if (r->f4C & 0x30) {
        k = 1;
        if ((r->f4C & 0x30) == 0x30) {
            k = 5;
        }
        (&(D_8009E960[D_8009CDDC] + k)->sp)->x0 = x;
        (&(D_8009E960[D_8009CDDC] + k)->sp)->y0 = y;
        func_80077AC4(D_800B0E38[D_8009CDDC] + 0x1C, (D_8009E960[D_8009CDDC] + k));
        x += dx;
    }
    if (*fl & 0xC0) {
        k = 2;
        if ((*fl & 0xC0) == 0xC0) {
            k = 6;
        }
        (&(D_8009E960[D_8009CDDC] + k)->sp)->x0 = x;
        (&(D_8009E960[D_8009CDDC] + k)->sp)->y0 = y;
        func_80077AC4(D_800B0E38[D_8009CDDC] + 0x1C, (D_8009E960[D_8009CDDC] + k));
        x += dx;
    }
    if (*fl & 3) {
        k = 3;
        if ((*fl & 3) == 3) {
            k = 7;
        }
        (&(D_8009E960[D_8009CDDC] + k)->sp)->x0 = x;
        (&(D_8009E960[D_8009CDDC] + k)->sp)->y0 = y;
        func_80077AC4(D_800B0E38[D_8009CDDC] + 0x1C, (D_8009E960[D_8009CDDC] + k));
        x += dx;
    }
    if (*fl & 0x1000) {
        (&(D_8009E960[D_8009CDDC] + 8)->sp)->x0 = x;
        (&(D_8009E960[D_8009CDDC] + 8)->sp)->y0 = y;
        func_80077AC4(D_800B0E38[D_8009CDDC] + 0x1C, (D_8009E960[D_8009CDDC] + 8));
        x += dx;
    }
    if (*fl & 0x100) {
        (&(D_8009E960[D_8009CDDC] + 9)->sp)->x0 = x;
        (&(D_8009E960[D_8009CDDC] + 9)->sp)->y0 = y;
        func_80077AC4(D_800B0E38[D_8009CDDC] + 0x1C, (D_8009E960[D_8009CDDC] + 9));
        x += dx;
    }
    if (*fl & 0x400) {
        (&(D_8009E960[D_8009CDDC] + 10)->sp)->x0 = x;
        (&(D_8009E960[D_8009CDDC] + 10)->sp)->y0 = y;
        func_80077AC4(D_800B0E38[D_8009CDDC] + 0x1C, (D_8009E960[D_8009CDDC] + 10));
        x += dx;
    }
    if (*fl & 0x800) {
        (&(D_8009E960[D_8009CDDC] + 11)->sp)->x0 = x;
        (&(D_8009E960[D_8009CDDC] + 11)->sp)->y0 = y;
        func_80077AC4(D_800B0E38[D_8009CDDC] + 0x1C, (D_8009E960[D_8009CDDC] + 11));
        x += dx;
    }
    if (*fl & 0x200) {
        (&(D_8009E960[D_8009CDDC] + 12)->sp)->x0 = x;
        (&(D_8009E960[D_8009CDDC] + 12)->sp)->y0 = y;
        func_80077AC4(D_800B0E38[D_8009CDDC] + 0x1C, (D_8009E960[D_8009CDDC] + 12));
    }
}
