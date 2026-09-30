extern int D_800A1820;
extern int D_800A1824;
extern int D_800A1828;
extern int D_800A182C;
extern int D_800A1830;
extern int D_800A1834;
extern int D_800A1838;
extern int D_800A183C;
extern int D_800A1840;
extern int D_800BCDA8;
extern int D_800BCDAC;
extern int D_800BCDB0;
extern int D_800BCDB4;
extern int D_800BCDB8;
extern int D_800BCDBC;
extern int D_800BCDC0;
extern int D_800BCDC4;
extern unsigned char D_800A0ED4[];
extern void func_800726F4();
extern void func_8007DD74();
extern void func_8007DD54();
extern void func_8007DD44();
extern int func_8004D4A0();
extern void func_8004298C();

void func_800405A4(int a0) {
    unsigned char *p;

    p = D_800A0ED4 + a0 * 0x418;
    switch (p[8]) {
    case 0:
        p[0] = 0;
        goto check;
    case 1:
        if (D_800A1820 == 0) {
            goto chk1824;
        }
        D_800A1820 = 0;
        if (p[0] & 1) {
            goto set4;
        }
        func_800726F4(D_800BCDB8);
        func_800726F4(D_800BCDBC);
        func_800726F4(D_800BCDC0);
        func_800726F4(D_800BCDC4);
        D_800A1834 = 0;
        D_800A1830 = 0;
        D_800A182C = 0;
        func_8007DD74(a0 << 4);
        p[8] = 2;
        return;
set4:
        p[8] = 4;
        goto tail;
chk1824:
        if (D_800A1824 != 0) {
            D_800A1824 = 0;
            p[8] = 0;
            p[0] = p[0] & 0xFB;
            goto tail;
        }
        if (D_800A1828 == 0) {
            return;
        }
        D_800A1828 = 0;
        func_800726F4(D_800BCDB8);
        func_800726F4(D_800BCDBC);
        func_800726F4(D_800BCDC0);
        func_800726F4(D_800BCDC4);
        D_800A1834 = 0;
        D_800A1830 = 0;
        D_800A182C = 0;
        func_8007DD74(a0 << 4);
        p[8] = 2;
        return;
    case 2:
        if (D_800A182C != 0) {
            D_800A182C = 0;
            func_800726F4(D_800BCDA8);
            func_800726F4(D_800BCDAC);
            func_800726F4(D_800BCDB0);
            func_800726F4(D_800BCDB4);
            D_800A1828 = 0;
            D_800A1824 = 0;
            D_800A1820 = 0;
            func_8007DD54(a0 << 4);
            p[8] = 3;
            return;
        }
        if (D_800A1830 == 0) {
            if (D_800A1834 == 0) {
                return;
            }
        }
        D_800A1830 = 0;
        D_800A1834 = 0;
        p[8] = 0;
        goto tail;
    case 3:
        if (D_800A1820 != 0) {
            D_800A1820 = 0;
            p[8] = 4;
            if (func_8004D4A0() == 0) {
                p[0] = p[0] | 1;
                func_8004298C(a0, 0);
            }
            goto tail;
        }
        if (D_800A1824 != 0) {
            D_800A1824 = 0;
            p[8] = 0;
            goto tail;
        }
        if (D_800A1828 == 0) {
            return;
        }
        D_800A1828 = 0;
        p[8] = 4;
        p[0] = p[0] | 4;
    tail:
        {
            int t = D_800A183C;
            D_800A1840 = 0;
            D_800A183C = (t == 0);
        }
        return;
    case 4:
        p[0] = p[0] | 1;
    check:
        if (D_800A1838 != 0) {
            return;
        }
        if (a0 != D_800A183C) {
            return;
        }
        {
            int c = D_800A1840;
            D_800A1840 = c - 1;
            if (c > 0) {
                return;
            }
        }
        func_800726F4(D_800BCDA8);
        func_800726F4(D_800BCDAC);
        func_800726F4(D_800BCDB0);
        func_800726F4(D_800BCDB4);
        D_800A1828 = 0;
        D_800A1824 = 0;
        D_800A1820 = 0;
        func_8007DD44(a0 << 4);
        p[8] = 1;
        return;
    }
}
