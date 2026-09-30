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

typedef struct Stat {
    unsigned char pad0[0x10];
    unsigned int pad : 4;
    unsigned int lv : 2;
} Stat;

typedef struct Rec {
    unsigned char pad0[0x68];
    Stat *f68;
} Rec;

extern Rec *D_8009D278;
extern int D_8009CDDC;
extern Pos D_8009E360[];
extern Ent D_8009E730[];
extern unsigned char *D_800B0E38[];
extern void func_80077AC4(unsigned char *, Ent *);
extern void func_80031760(int, signed char);
extern void func_80031D6C(int);
extern void func_8003205C(void);
extern void func_800323C8(signed char);
extern void func_800325DC(void);
extern void func_800327D8(short, int);
extern void func_80031E68(void);

void func_800314E4(int a0, signed char mode, signed char a2, short a3)
{
    Sprt *s;

    if (mode != 8) {
        if (mode == 1 || mode == 2) {
            D_8009E360[D_8009CDDC].x = 120;
            D_8009E360[D_8009CDDC].y = 104;
        } else {
            func_80031760(a0, a2);
        }
        switch (mode) {
        case 0:
            func_80031D6C(40);
            func_8003205C();
            func_800325DC();
            func_800327D8(a3, 30);
            break;
        case 1:
        case 2:
            func_80031D6C(32);
            func_800323C8(mode);
            func_800325DC();
            break;
        case 3:
            func_80031D6C(28);
            (&D_8009E730[D_8009CDDC].sp)->x0 = D_8009E360[D_8009CDDC].x + 28;
            (&D_8009E730[D_8009CDDC].sp)->y0 = D_8009E360[D_8009CDDC].y + 8;
            func_80077AC4(D_800B0E38[D_8009CDDC] + 16, &D_8009E730[D_8009CDDC]);
            func_800327D8(a3, 18);
            break;
        case 4:
            func_80031D6C(40);
            func_800323C8(mode);
            func_800325DC();
            func_800327D8(a3, 30);
            break;
        case 5:
        case 6:
        case 7:
            func_80031D6C(32);
            func_800323C8(mode);
            func_800327D8(a3, 22);
            break;
        }
        } else {
        D_8009E360[D_8009CDDC].x = 120;
        D_8009E360[D_8009CDDC].y = 108;
        func_80031D6C(24);
        func_800323C8(8);
    }
    if (D_8009D278->f68->lv >= 2) {
        func_80031E68();
    }
}
