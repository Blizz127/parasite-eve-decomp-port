typedef struct W {
    unsigned char pad0[0x44];
    int f44;
} W;

typedef struct Rec {
    unsigned char pad0[7];
    unsigned char b7;
    unsigned char b8;
    unsigned char b9;
    unsigned char padA[4];
    short hE;
    short h10;
    short h12;
} Rec;

extern int D_8009CF30;
extern int D_8009CF1C;
extern int D_8009CF18;
extern signed char D_800C0E20[];
extern signed char D_800C0E22[];
extern W *func_80062A34(int, int);
extern int func_80063428(W *);
extern int func_800556E8(int);
extern void func_80059EC8(int, int);
extern int func_80059F08(int);
extern Rec *func_8005332C(int);
extern W *func_80062CC4(void);
extern void func_8004551C(Rec *);
extern void func_80045670(Rec *, Rec *);
extern void func_8005E8A4(int, int);
extern void func_8005FDF0(int);
extern void func_8005FF28(int);
extern void func_8005EB64(int);

void func_80045A98(void)
{
    W *w;
    Rec *rec;
    Rec *rec2;

    w = func_80062A34(2, 13);
    if (w != 0) {
        if (w->f44 >= 0) {
            func_80059EC8(0, func_800556E8(func_80063428(w)));
        }
    } else if (D_8009CF30 != 0) {
        w = func_80062A34(2, 16);
        if (w != 0 && w->f44 >= 0) {
            func_80059EC8(0, func_800556E8(func_80063428(w)));
        }
    } else if (D_8009CF1C == 0) {
        func_80059EC8(0, D_8009CF18 ? D_800C0E20[0] : D_800C0E22[0]);
    }
    rec = func_8005332C(D_8009CF1C ? func_80059F08(0) : (D_8009CF18 ? D_800C0E20[0] : D_800C0E22[0]));
    if (func_80062A34(2, 7) == func_80062CC4()) {
        rec2 = func_8005332C(func_80059F08(1));
        func_8004551C(D_8009CF1C ? rec2 : rec);
        func_80045670(rec, rec2);
    } else {
        func_8004551C(rec);
        if (rec != 0) {
            func_8005E8A4(42, -12);
            func_8005FDF0(rec->b9);
            func_8005E8A4(5, 0);
            func_8005FF28(rec->h12);
            func_8005E8A4(-45, -14);
            func_8005FDF0(rec->b8);
            func_8005E8A4(5, 0);
            func_8005FF28(rec->h10);
            func_8005E8A4(-45, -14);
            func_8005FDF0(rec->b7);
            func_8005E8A4(5, 0);
            func_8005FF28(rec->hE);
            func_8005E8A4(-45, -10);
            func_8005EB64(135);
            func_8005E8A4(25, 0);
            func_8005EB64(136);
        }
    }
}
