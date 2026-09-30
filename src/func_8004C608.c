typedef struct Menu {
    unsigned char pad0[0x34];
    int f34;
} Menu;

typedef struct Help {
    unsigned char pad0[0x24];
    unsigned int f24;
} Help;

extern int D_8009CEF0;
extern int D_8009CF18;
extern int D_8009CF8C;
extern int D_8009CF1C;
extern int D_8009CEFC;
extern unsigned char *D_8009CF20;
extern int D_8009CF50;
extern int D_8009CFF8;
extern int D_8009CF0C;
extern signed char D_800C0E20;
extern signed char D_800C0E22;
extern unsigned char D_8009234C[];

extern Help *func_80062CC4(void);
extern void func_8005E8C4(void);
extern void func_8005E8A4(int, int);
extern int func_80052894(int);
extern void func_8006006C(int, int);
extern void func_8005EB64(int);
extern void func_8005E914(void);
extern int func_80062A34(int, int);
extern int func_80063428(int);
extern int func_8005B89C(void);
extern unsigned char *func_8005332C(int);
extern char *func_8005DCEC(int);
extern int func_8005415C(int);
extern char *func_8005DC4C(int);
extern void func_8005E968(int);
extern unsigned char *func_80058BBC(int);
extern int func_80058E08(int);
extern int func_80059F08(int);
extern int func_800556E8(int);
extern int func_80054288(void);
extern int func_80057ED8(int);
extern void func_80052E30(int);
extern int func_800404A8(void);
extern void func_8005F5B8(int);
extern int func_80042770(int);
extern void func_8005EB58(int);
extern int func_8003FFBC(void);
extern char *func_8005DD3C(int);
extern void func_8005F27C(char *);

void func_8004C608(Menu *m)
{
    char *txt;
    Help *h;
    int sel;
    int bits;
    int k;
    int f;
    int c;
    unsigned char *p;

    txt = 0;
    h = func_80062CC4();
    func_8005E8C4();
    func_8005E8A4(m->f34 - 0x27, 1);
    func_8006006C(func_80052894(2), 1);
    func_8005E8A4(-0x21, 0);
    func_8005EB64(0x8C);
    func_8005E914();
    if (h == 0) {
        goto show;
    }
    sel = func_80063428(func_80062A34(2, h->f24));
    switch (h->f24) {
    case 0:
        if (func_8005B89C() != 0) {
            bits = D_8009CEF0 & 0x1F;
        } else {
            bits = D_8009CEF0 & 0x1EF;
        }
        k = -1;
    loop:
        if (sel >= 0) {
            sel -= bits & 1;
            k++;
            bits >>= 1;
            if (k < 9) {
                goto loop;
            }
        }
        txt = func_8005DD3C(k + 0x28);
        break;
    case 5:
        if (sel < 0) {
            break;
        }
        p = func_8005332C(D_8009CF18 ? D_800C0E20 : D_800C0E22);
        if (p != 0) {
            txt = func_8005DCEC(p[4] - 1);
        }
        break;
    case 1:
        k = D_8009CF8C;
        if (k >= 0) {
            goto cf;
        }
        p = func_8005332C(sel);
        if (p != 0) {
            txt = func_8005DCEC(p[4] - 1);
        }
        break;
    case 51:
        k = D_8009CF8C;
        if (k >= 0) {
            goto cf;
        }
        p = func_8005332C(func_80058E08(sel));
        if (p != 0) {
            txt = func_8005DCEC(p[4] - 1);
        }
        break;
    case 52:
        k = D_8009CF8C;
        if (k >= 0) {
        cf:
            f = 0;
            if (func_8005415C(k) < 0x13 || func_8005415C(D_8009CF8C) >= 0x16) {
                f = 1;
            }
            txt = func_8005DC4C(f | 0xE);
            func_8005E968(0x408040);
            break;
        }
        p = func_80058BBC(sel);
        if (p != 0) {
            txt = func_8005DCEC(p[4] - 1);
        }
        break;
    case 7:
        if (D_8009CF1C != 0) {
            sel = func_80059F08(1);
        } else {
            sel = func_800556E8(sel);
        }
        p = func_8005332C(sel);
        if (p != 0) {
            txt = func_8005DCEC(p[4] - 1);
        }
        break;
    case 8:
        if (sel < func_80054288()) {
            txt = func_8005DCEC(func_800556E8(sel) + 0xEB);
        }
        break;
    case 12:
        txt = func_8005DD3C(sel + 0x4F);
        break;
    case 13:
    case 16:
        if (D_8009CEFC != 0) {
            txt = func_8005DCEC(func_80057ED8(sel) - 1);
            break;
        }
        p = func_8005332C(func_800556E8(sel));
        if (p != 0) {
            txt = func_8005DCEC(p[4] - 1);
        }
        break;
    case 14:
        func_80052E30(0);
        p = func_8005332C(sel);
        if (p != 0) {
            txt = func_8005DCEC(p[4] - 1);
        }
        break;
    case 6:
        c = (D_8009CF20 + sel)[0x15] & 0x1F;
        if (c != 0) {
            txt = func_8005DD3C(c + (1 - D_8009CF18) * 20 - 1);
        }
        break;
    case 11:
        c = (func_8005332C(func_80059F08(1)) + sel)[0x15] & 0x1F;
        if (c != 0) {
            txt = func_8005DD3C(c + (1 - D_8009CF18) * 20 - 1);
        }
        break;
    case 17:
        txt = func_8005DC4C(0x27);
        break;
    case 27:
    case 28:
        txt = func_8005DD3C(0x52);
        break;
    case 29:
        {
            int t = D_8009CF18 * 3 - 0x56;
            txt = func_8005DD3C(sel - t);
        }
        break;
    case 32:
        if (sel < 4) {
            txt = func_8005DD3C(sel + 0x31);
        } else {
            txt = 0;
        }
        break;
    case 33:
        txt = func_8005DD3C(sel + 0x3A);
        break;
    case 35:
        txt = func_8005DD3C(sel + 0x3D);
        break;
    case 36:
        if (func_800404A8() != 0) {
            func_8005E8A4(6, 5);
            func_8005F5B8(0x4C);
            func_8005E8A4(0, 0x10);
            func_8005F5B8(0x4D);
            break;
        }
        f = 0;
        if (func_80042770(0) != 0 || func_80042770(1) != 0) {
            f = 1;
        }
        txt = func_8005DC4C(f | 0x4E);
        break;
    case 37:
    case 38:
        if (D_8009CF50 != 0) {
            if (D_8009CFF8 != 0) {
                func_8005EB58(0);
                func_8005E8A4(6, 5);
                func_8005F5B8(0x58);
                func_8005E8A4(0, 0x10);
                func_8005F5B8(0x59);
                break;
            }
            txt = func_8005DC4C(0x5C);
            break;
        }
        txt = func_8005DC4C(func_8003FFBC() ? 0x57 : 0x5B);
        break;
    case 39:
        func_8005EB58(0);
        func_8005E8A4(6, 5);
        func_8005F5B8(0x50);
        func_8005E8A4(0, 0x10);
        func_8005F5B8(0x51);
        break;
    case 46:
        txt = func_8005DD3C(sel + 0x3F);
        break;
    case 48:
        txt = func_8005DD3C(sel + 0x38);
        break;
    case 49:
        txt = func_8005DD3C(sel + 0x42);
        break;
    case 50:
        {
            unsigned char *b = D_8009234C;
            unsigned char *q;
            if (D_8009CF0C == 1 && sel == 2) {
                q = b + 4;
            } else {
                q = (unsigned char *)(sel + (int)b);
            }
            txt = func_8005DD3C(*q);
        }
        break;
    case 56:
        func_8005EB58(0);
        txt = func_8005DD3C(0x44);
        break;
    case 58:
        txt = func_8005DD3C(sel + 0x35);
        break;
    case 59:
        txt = func_8005DD3C(D_8009CF0C == 2 ? sel + 0x4D : sel + 0x4B);
        break;
    case 60:
        {
            int t = D_8009CF18 * 3 - 0x48;
            txt = func_8005DD3C(sel - t);
        }
        break;
    }
show:
    if (txt != 0) {
        func_8005E8A4(6, 5);
        func_8005F27C(txt);
        func_8005E968(0x808080);
    }
}
