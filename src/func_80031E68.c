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

typedef struct Pos {
    unsigned short x;
    unsigned short y;
    unsigned char pad4[0x2C];
} Pos;

typedef struct Inner {
    unsigned char pad0[0x10];
    unsigned int lo : 4;
    unsigned int n : 2;
} Inner;

typedef struct Rec {
    unsigned char pad0[0x68];
    Inner *f68;
} Rec;

extern Rec *D_8009D278;
extern signed char D_8009D2D8;
extern int D_8009CDDC;
extern Ent D_8009E3B8[][3];
extern Pos D_8009E360[];
extern unsigned char *D_800B0E38[];
extern void func_80077AC4(unsigned char *, Ent *);

void func_80031E68(void)
{
    unsigned char i;

    for (i = 0; i < D_8009D278->f68->n; i++) {
        if (i == D_8009D2D8 - 1) {
            (&(D_8009E3B8[D_8009CDDC] + i)->sp)->u0 = i * 24 + 104;
            (&(D_8009E3B8[D_8009CDDC] + i)->sp)->v0 = 224;
        } else {
            (&(D_8009E3B8[D_8009CDDC] + i)->sp)->u0 = 176;
            (&(D_8009E3B8[D_8009CDDC] + i)->sp)->v0 = 224;
        }
        (&(D_8009E3B8[D_8009CDDC] + i)->sp)->x0 = D_8009E360[D_8009CDDC].x + i * 24;
        (&(D_8009E3B8[D_8009CDDC] + i)->sp)->y0 = D_8009E360[D_8009CDDC].y - 8;
        func_80077AC4(D_800B0E38[D_8009CDDC] + 28, D_8009E3B8[D_8009CDDC] + i);
    }
}
