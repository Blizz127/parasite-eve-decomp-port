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

typedef struct Panel {
    unsigned char pad[0x70];
} Panel;

typedef struct Stat {
    unsigned char pad0[0xC];
    unsigned int f0C;
    unsigned int f10;
} Stat;

typedef struct Rec {
    unsigned char pad0[0x68];
    Stat *f68;
} Rec;

typedef struct Cmd {
    short type;
    unsigned char pad2[6];
} Cmd;

extern Rec *D_8009D278;
extern int D_8009CDDC;
extern Pos D_8009E360[];
extern Ent D_8009E768[];
extern Panel D_8009E7A0[];
extern Cmd D_800BE834[];
extern unsigned char *D_800B0E38[];
extern int func_80056C14(int);
extern void func_80077AC4(unsigned char *, Ent *);
extern signed char func_80021054(void);
extern void func_800328DC(Panel *, short, short, short, int);

void func_800325DC(void)
{
    signed char i;
    short n;
    unsigned short y;
    int r;
    int t;

    i = 0;
    y = D_8009E360[D_8009CDDC].y + 22;
    r = func_80056C14(((D_8009D278->f68->f0C >> 20) & 3) - 1);
    n = r + (*(unsigned short *)&D_8009D278->f68->f0C & 0x3FF);
    (&(D_8009E768 + D_8009CDDC)->sp)->x0 = D_8009E360[D_8009CDDC].x + 8;
    (&(D_8009E768 + D_8009CDDC)->sp)->y0 = y;
    func_80077AC4(D_800B0E38[D_8009CDDC] + 16, D_8009E768 + D_8009CDDC);
    for (; i < func_80021054(); i++) {
        if (D_800BE834[i].type == 1) {
            n--;
        } else if (D_800BE834[i].type == 2) {
            t = D_8009D278->f68->f10 & 0xC0;
            if (t == 0xC0) {
                n -= *(unsigned char *)&D_8009D278->f68->f10 & 0xF;
            } else if (t == 0x40) {
                n--;
            }
        } else if (D_800BE834[i].type == 393) {
            n--;
        }
    }
    if (n < 0) {
        n = 0;
    }
    func_800328DC(&D_8009E7A0[D_8009CDDC], D_8009E360[D_8009CDDC].x + 64, y - 1, n, 0);
}
