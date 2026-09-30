typedef struct Rec {
    int w[9];
} Rec;

extern Rec *D_8009D014;
extern Rec D_800A1AA0[];
extern short D_800C0E48[];
extern signed char D_800C0E20[];
extern signed char D_800C0E22[];
extern void func_80053D2C(int);
extern int func_800533D4(int);

void func_8005112C(void)
{
    Rec *p;
    short *e;

    if (D_800A1AA0 < D_8009D014) {
        p = --D_8009D014;
        switch (p->w[0]) {
        case 0:
            e = &D_800C0E48[p->w[2]];
            if (*e == 0) {
                *e = p->w[1];
            } else {
                func_80053D2C(p->w[1]);
            }
            break;
        case 1:
            break;
        case 2:
            D_800C0E20[0] = func_800533D4(p->w[1]);
            break;
        case 3:
            D_800C0E22[0] = p->w[1] ? func_800533D4(p->w[1]) : -1;
            break;
        case 4:
            *(short *)(p->w[1] + 0xA) = p->w[5];
            *(short *)(p->w[2] + 0xA) = p->w[7];
            *(short *)(p->w[3] + 0xA) = p->w[8];
            break;
        }
    }
}
