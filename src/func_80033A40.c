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

typedef struct Tile {
    unsigned int tag;
    unsigned char r0, g0, b0, code;
    short x0, y0;
    short w, h;
} Tile;

typedef struct TEnt {
    unsigned int tp[2];
    Tile t;
} TEnt;

typedef struct PolyF4 {
    unsigned int tag;
    unsigned char r0, g0, b0, code;
    short x0, y0;
    unsigned char r1, g1, b1, p1;
    short x1, y1;
    unsigned char r2, g2, b2, p2;
    short x2, y2;
    unsigned char r3, g3, b3, p3;
    short x3, y3;
} PolyF4;

/* D_8009D278 record: f8 is a 16.16 word whose integer half is read at +0xA */
typedef struct St {
    unsigned char pad0[8];
    int f8;
    short fC;
    short padE;
    unsigned short f10;
    unsigned char pad12[0x1C - 0x12];
    short f1C;
    unsigned char pad1E[0x28 - 0x1E];
    int f28;
} St;

extern St *D_8009D278;
extern St **D_8009D254;
extern unsigned int D_8009D1A0;
extern int D_8009CDDC;
extern unsigned short D_8009CE84[2];
extern Tile D_8009E098[];
extern PolyF4 D_800B00E8[];
extern PolyF4 D_800B0130[][2];
extern Ent D_800B6920[];
extern Ent D_8009E0B8[];
extern Ent D_8009E320[];
extern TEnt D_8009E068[];
extern unsigned char *D_800B0E38[];
extern void func_80077AC4(void *, void *);
extern int func_800438E0(void);
extern void func_80034104(short, short);
extern void func_800334AC(void);

void func_80033A40(void)
{
    int w1;
    int w2;

    if (!(D_8009D1A0 & 2)) {
        D_8009D278 = *D_8009D254;
    }
    if (D_8009D278->f10 > 9000) {
        D_8009D278->f10 = 9000;
    }
    if (D_8009D278->f8 > D_8009D278->f28) {
        D_8009D278->f8 = D_8009D278->f28;
    } else if (D_8009D278->f8 < 0) {
        D_8009D278->f8 = 0;
    }
    if (D_8009D278->f1C < D_8009D278->fC) {
        D_8009D278->fC = D_8009D278->f1C;
    }
    w2 = (*(short *)((char *)D_8009D278 + 0xA) * 56) / *(short *)((char *)D_8009D278 + 0x2A);
    w1 = (D_8009D278->f10 * 56) / 9000;
    if (D_8009D1A0 & 2) {
        (&D_8009E098[D_8009CDDC])->x0 = D_8009CE84[0] + 8;
        (&D_8009E098[D_8009CDDC])->y0 = D_8009CE84[1] + 4;
        (&D_800B00E8[D_8009CDDC])->x0 = D_8009CE84[0] + 8;
        (&D_800B00E8[D_8009CDDC])->y0 = D_8009CE84[1] + 4;
        (&D_800B00E8[D_8009CDDC])->x1 = D_8009CE84[0] + 8 + w1;
        (&D_800B00E8[D_8009CDDC])->y1 = D_8009CE84[1] + 4;
        (&D_800B00E8[D_8009CDDC])->x2 = D_8009CE84[0] + 8;
        (&D_800B00E8[D_8009CDDC])->y2 = D_8009CE84[1] + 7;
        (&D_800B00E8[D_8009CDDC])->x3 = D_8009CE84[0] + 8 + w1;
        (&D_800B00E8[D_8009CDDC])->y3 = D_8009CE84[1] + 7;
        (&D_800B6920[D_8009CDDC].sp)->x0 = D_800B00E8[D_8009CDDC].x1;
        (&D_800B6920[D_8009CDDC].sp)->y0 = D_800B00E8[D_8009CDDC].y1 - 2;
        (&D_8009E0B8[D_8009CDDC].sp)->x0 = D_8009CE84[0] + 0x44;
        (&D_8009E0B8[D_8009CDDC].sp)->y0 = D_8009CE84[1] + 3;
        func_80077AC4(D_800B0E38[D_8009CDDC] + 0x18, &D_8009E098[D_8009CDDC]);
        func_80077AC4(D_800B0E38[D_8009CDDC] + 0x14, &D_800B00E8[D_8009CDDC]);
        func_80077AC4(D_800B0E38[D_8009CDDC] + 0x10, &D_800B6920[D_8009CDDC]);
        func_80077AC4(D_800B0E38[D_8009CDDC] + 0x10, &D_8009E0B8[D_8009CDDC]);
    }
    if (func_800438E0() & 2) {
        (&D_800B0130[D_8009CDDC][0])->x0 = D_8009CE84[0] + 8;
        (&D_800B0130[D_8009CDDC][0])->y0 = D_8009CE84[1] + 0x19;
        (&D_800B0130[D_8009CDDC][0])->x1 = D_8009CE84[0] + 8 + w2;
        (&D_800B0130[D_8009CDDC][0])->y1 = D_8009CE84[1] + 0x19;
        (&D_800B0130[D_8009CDDC][0])->x2 = D_8009CE84[0] + 8;
        (&D_800B0130[D_8009CDDC][0])->y2 = D_8009CE84[1] + 0x1C;
        (&D_800B0130[D_8009CDDC][0])->x3 = D_8009CE84[0] + 8 + w2;
        (&D_800B0130[D_8009CDDC][0])->y3 = D_8009CE84[1] + 0x1C;
        (&D_800B0130[D_8009CDDC][1])->x0 = D_800B0130[D_8009CDDC][0].x1;
        (&D_800B0130[D_8009CDDC][1])->y0 = D_8009CE84[1] + 0x19;
        (&D_800B0130[D_8009CDDC][1])->x1 = 0x38 - w2 + D_800B0130[D_8009CDDC][0].x1;
        (&D_800B0130[D_8009CDDC][1])->y1 = D_8009CE84[1] + 0x19;
        (&D_800B0130[D_8009CDDC][1])->x2 = D_800B0130[D_8009CDDC][0].x1;
        (&D_800B0130[D_8009CDDC][1])->y2 = D_8009CE84[1] + 0x1C;
        (&D_800B0130[D_8009CDDC][1])->x3 = 0x38 - w2 + D_800B0130[D_8009CDDC][0].x1;
        (&D_800B0130[D_8009CDDC][1])->y3 = D_8009CE84[1] + 0x1C;
        (&D_8009E320[D_8009CDDC].sp)->x0 = D_8009CE84[0] + 0x44;
        (&D_8009E320[D_8009CDDC].sp)->y0 = D_8009CE84[1] + 0x18;
        func_80077AC4(D_800B0E38[D_8009CDDC] + 0x14, &D_800B0130[D_8009CDDC][0]);
        func_80077AC4(D_800B0E38[D_8009CDDC] + 0x14, &D_800B0130[D_8009CDDC][1]);
        func_80077AC4(D_800B0E38[D_8009CDDC] + 0x10, &D_8009E320[D_8009CDDC]);
        if (D_8009D1A0 & 2) {
            (&D_8009E068[D_8009CDDC].t)->x0 = D_8009CE84[0];
            (&D_8009E068[D_8009CDDC].t)->y0 = D_8009CE84[1];
            (&D_8009E068[D_8009CDDC].t)->w = 0x54;
            (&D_8009E068[D_8009CDDC].t)->h = 0x20;
        } else {
            (&D_8009E068[D_8009CDDC].t)->x0 = D_8009CE84[0];
            (&D_8009E068[D_8009CDDC].t)->y0 = D_8009CE84[1] + 7;
            (&D_8009E068[D_8009CDDC].t)->w = 0x54;
            (&D_8009E068[D_8009CDDC].t)->h = 0x19;
        }
    } else {
        if (D_8009D1A0 & 2) {
            (&D_8009E068[D_8009CDDC].t)->x0 = D_8009CE84[0];
            (&D_8009E068[D_8009CDDC].t)->y0 = D_8009CE84[1];
            (&D_8009E068[D_8009CDDC].t)->w = 0x54;
            (&D_8009E068[D_8009CDDC].t)->h = 0x19;
        } else {
            (&D_8009E068[D_8009CDDC].t)->x0 = D_8009CE84[0];
            (&D_8009E068[D_8009CDDC].t)->y0 = D_8009CE84[1] + 7;
            (&D_8009E068[D_8009CDDC].t)->w = 0x54;
            (&D_8009E068[D_8009CDDC].t)->h = 0x12;
        }
    }
    func_80077AC4(D_800B0E38[D_8009CDDC] + 0x1C, &D_8009E068[D_8009CDDC]);
    func_80034104(D_8009D278->f1C, D_8009D278->fC);
    func_800334AC();
}
