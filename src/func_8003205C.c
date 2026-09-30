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
    unsigned char pad0[0xC];
    unsigned int pc : 20;
    unsigned int mode : 2;
    unsigned int pc2 : 10;
    unsigned int lo : 6;
    unsigned int b6 : 2;
} Inner;

typedef struct Rec {
    unsigned char pad0[0x68];
    Inner *f68;
} Rec;

extern Rec *D_8009D278;
extern unsigned char D_8009D1DC;
extern int D_8009CDDC;
extern Ent D_8009E500[][10];
extern Pos D_8009E360[];
extern unsigned char *D_800B0E38[];
extern unsigned short func_80077AA4(int, int);
extern void func_80077AC4(unsigned char *, Ent *);

void func_8003205C(void)
{
    short i;
    short x;
    short y;

    y = D_8009E360[D_8009CDDC].y + 4;
    if (D_8009D278->f68->mode == 2) {
        (&(D_8009E500[D_8009CDDC] + 0)->sp)->w = 8;
        (&(D_8009E500[D_8009CDDC] + 0)->sp)->h = 16;
        (&(D_8009E500[D_8009CDDC] + 0)->sp)->u0 = 224;
        (&(D_8009E500[D_8009CDDC] + 0)->sp)->v0 = 224;
        (&(D_8009E500[D_8009CDDC] + 0)->sp)->x0 = D_8009E360[D_8009CDDC].x + 60;
        (&(D_8009E500[D_8009CDDC] + 0)->sp)->y0 = y;
        D_8009E500[D_8009CDDC][0].sp.clut = func_80077AA4(0x130, 0x1FD);
        func_80077AC4(D_800B0E38[D_8009CDDC] + 16, D_8009E500[D_8009CDDC] + 0);
    } else {
        x = D_8009E360[D_8009CDDC].x + 64;
        for (i = 0; i < D_8009D1DC; i++) {
            (&(D_8009E500[D_8009CDDC] + i)->sp)->w = 4;
            (&(D_8009E500[D_8009CDDC] + i)->sp)->h = 16;
            if (D_8009D278->f68->mode == 3) {
                (&(D_8009E500[D_8009CDDC] + i)->sp)->u0 = 216;
                (&(D_8009E500[D_8009CDDC] + i)->sp)->v0 = 224;
                D_8009E500[D_8009CDDC][i].sp.clut = func_80077AA4(0x130, 0x1FB);
            } else if (D_8009D278->f68->b6 == 2) {
                (&(D_8009E500[D_8009CDDC] + i)->sp)->u0 = 220;
                (&(D_8009E500[D_8009CDDC] + i)->sp)->v0 = 224;
                D_8009E500[D_8009CDDC][i].sp.clut = func_80077AA4(0x130, 0x1FC);
            } else {
                (&(D_8009E500[D_8009CDDC] + i)->sp)->u0 = 212;
                (&(D_8009E500[D_8009CDDC] + i)->sp)->v0 = 224;
                D_8009E500[D_8009CDDC][i].sp.clut = func_80077AA4(0x130, 0x1FA);
            }
            (&(D_8009E500[D_8009CDDC] + i)->sp)->x0 = x;
            (&(D_8009E500[D_8009CDDC] + i)->sp)->y0 = y;
            func_80077AC4(D_800B0E38[D_8009CDDC] + 16, D_8009E500[D_8009CDDC] + i);
            x -= 6;
        }
    }
}
