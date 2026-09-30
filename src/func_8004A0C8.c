typedef struct {
    unsigned char pad0[6];
    unsigned char f6;
    unsigned char f7;
    unsigned char f8;
    unsigned char f9;
    unsigned char padA[4];
    short fE;
    short f10;
    short f12;
    unsigned char f14;
} Item;

typedef struct {
    unsigned char pad0[0x44];
    int f44;
} Win;

extern int D_8009CF30;
extern int D_8009CF1C;
extern int D_8009CF18;
extern int D_8009CF34;
extern int D_8009CF38;
extern int D_8009CFC4;
extern int D_8009CFC8;
extern int D_8009CFCC;
extern signed char D_800C0E20;
extern signed char D_800C0E22;

extern Win *func_80062A34(int, int);
extern int func_80063428(Win *);
extern int func_800556E8(int);
extern void func_80059EC8(int, int);
extern int func_80059F08(int);
extern void func_8005E8A4(int, int);
extern void func_800536B8(int);
extern Item *func_8005332C(int);
extern void func_8004551C(Item *);
extern void func_8005FDF0(int);
extern void func_8005FF28(int);
extern void func_8005EB64(int);
extern void func_8005E988(int, int);
extern void func_8005E968(int);
extern void func_800631AC(Win *);
extern void func_80063198(Win *);

#define STAT(f, k) (((it->f + ((D_8009CFC8 == (k)) ? D_8009CFCC : 0)) < 1000) ? (it->f + ((D_8009CFC8 == (k)) ? D_8009CFCC : 0)) : 999)

void func_8004A0C8(void)
{
    Win *w;
    Item *it;
    int id;
    int v;

    w = func_80062A34(2, 0xD);
    if (w != 0) {
        if (w->f44 >= 0) {
            func_80059EC8(0, func_800556E8(func_80063428(w)));
        }
    } else if (D_8009CF30 != 0) {
        w = func_80062A34(2, 0x10);
        if (w != 0) {
            if (w->f44 >= 0) {
                func_80059EC8(0, func_800556E8(func_80063428(w)));
            }
        }
    } else if (D_8009CF1C == 0) {
        if (D_8009CF18 != 0) {
            func_80059EC8(0, D_800C0E20);
        } else {
            func_80059EC8(0, D_800C0E22);
        }
    }
    id = func_80059F08(0);
    func_8005E8A4(4, 4);
    func_800536B8(id);
    func_8005E8A4(-4, -4);
    it = func_8005332C(id);
    D_8009CF18 = it->f6 != 9;
    func_8004551C(it);
    if (D_8009CF1C != 0 || D_8009CF30 != 0 || D_8009CF34 != 0 || D_8009CF38 != 0) {
        if (it != 0) {
            func_8005E8A4(0x2A, -0xC);
            func_8005FDF0(it->f9);
            func_8005E8A4(5, 0);
            func_8005FF28(it->f12);
            func_8005E8A4(-0x2D, -0xE);
            func_8005FDF0(it->f8);
            func_8005E8A4(5, 0);
            func_8005FF28(it->f10);
            func_8005E8A4(-0x2D, -0xE);
            func_8005FDF0(it->f7);
            func_8005E8A4(5, 0);
            func_8005FF28(it->fE);
            func_8005E8A4(-0x2D, -0xA);
            func_8005EB64(0x87);
            func_8005E8A4(0x19, 0);
            func_8005EB64(0x88);
        }
    }
    if (D_8009CFC4 >= 0) {
        func_8005E8A4(0x28, -0x32);
        func_8005EB64(0x88);
        func_8005E8A4(0, 10);
        func_8005FF28(it->fE);
        func_8005E8A4(-0x14, 0xE);
        func_8005FF28(it->f10);
        func_8005E8A4(-0x14, 0xE);
        func_8005FF28(it->f12);
        func_8005EB64(0x22);
        func_8005E8A4(0, -0xE);
        func_8005EB64(0x22);
        func_8005E8A4(0, -0xE);
        func_8005EB64(0x22);
        func_8005E8A4(10, -10);
        func_8005EB64(0x88);
        func_8005E8A4(0, 10);
        v = STAT(fE, 0);
        func_8005E988(it->fE, v);
        func_8005FF28(v);
        func_8005E8A4(-0x14, 0xE);
        v = STAT(f10, 1);
        func_8005E988(it->f10, v);
        func_8005FF28(v);
        func_8005E8A4(-0x14, 0xE);
        v = STAT(f12, 2);
        func_8005E988(it->f12, v);
        func_8005FF28(v);
        func_8005E968(0x808080);
    }
    (it->f14 ? func_80063198 : func_800631AC)(func_80062A34(1, 0xB));
}
