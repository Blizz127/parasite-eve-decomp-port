extern unsigned int D_8009D1A0;
extern unsigned int D_8009D1F4;
extern unsigned int D_8009D26C;
extern unsigned int D_8009D1E4;
extern unsigned int D_8009D238;
extern unsigned int D_8009D2D4;
extern int D_8009D2A8;
extern unsigned int D_8009D280;
extern unsigned short D_800BE9A0;
extern unsigned short D_800BE9A2;
extern unsigned char D_800BE9A6;
extern unsigned char D_800BE9A7;
extern unsigned char D_8009D1C0[];
extern unsigned char D_800921F8[];
extern unsigned int D_800A76F0[];
extern int D_800A7770[];
extern unsigned int D_80092200[];
extern unsigned int D_800B0CD8;
extern signed char D_800B0DBF;

extern int func_800825C0(int);
extern void func_80082974(int, void *, int);
extern int func_80082680(int, int, int);
extern void func_8008292C(int, int, int);
extern void func_800828F4(int, void *);
extern int func_80062A34(int, int);

void func_8003EB04(void)
{
    unsigned int f;
    unsigned int f0;
    int t;
    int v;
    int b;
    unsigned short m;
    unsigned short i;
    unsigned int c;
    unsigned int e;
    int k;
    unsigned int g;
    unsigned char x;

    if (func_800825C0(0) == 0) {
        f0 = D_8009D1A0;
        if (!(f0 & 0x4001)) {
            D_8009D1F4 = 4;
            D_8009D26C = 4;
            D_8009D1E4 = 0;
            D_8009D1A0 = f0 | 0x4000;
            return;
        }
        D_8009D1A0 = f0 | 0x4000;
    }
    t = D_800BE9A0 & 0xF000;
    if (t == 0x4000 || t == 0x7000) {
    if (D_8009D1A0 & 0x4000) {
        v = func_800825C0(0);
        if (v == 2) {
            goto clear;
        }
        if (v == 1) {
            func_80082974(0, D_8009D1C0, 2);
        } else if (v == 6) {
            if (func_80082680(0, 2, 0) != 0) {
                if (!(D_8009D1A0 & 0x8000)) {
                    func_8008292C(0, 1, 0);
                    D_8009D1A0 |= 0x8000;
                } else {
                    func_800828F4(0, D_800921F8);
                clear:
                    D_8009D1A0 &= ~0x4000;
                }
            }
        }
    }
    m = ~D_800BE9A2 & ~0x6000;
    D_8009D238 = D_8009D26C;
    b = ~D_800BE9A2;
    if (b & 0x2000) {
        m |= 0x4000;
    }
    if (b & 0x4000) {
        m |= 0x2000;
    }
    D_8009D26C = 0;
    for (i = 0; i < 32; i++) {
        if (m & D_800A76F0[i]) {
            D_8009D26C |= 1 << i;
            D_800A7770[i]++;
        } else {
            D_800A7770[i] = 0;
        }
    }
    f = D_8009D1A0;
    if (f & 1) {
        c = D_8009D26C;
        if ((int)c < 0) {
            e = ((c ^ D_8009D2D4) & c) & 0x7000007E;
            if (e != 0) {
                k = D_8009D2A8;
                if ((e & D_80092200[k]) == D_80092200[k]) {
                    D_8009D2A8 = k + 1;
                    if (k + 1 == 9) {
                        D_8009D280 = 0xAA108448;
                        D_8009D1A0 = f | 0x12000;
                    }
                } else {
                    D_8009D2A8 = 0;
                }
            }
        } else {
            D_8009D2A8 = 0;
        }
        D_8009D2D4 = D_8009D26C;
    }
    g = D_800B0CD8;
    if (g & 0x400) {
        D_8009D26C &= 0x40FFDC7F;
    }
    if (g & 0x200) {
        D_8009D26C &= 0x60FFDB04;
        if (D_800B0DBF != 1) {
            D_8009D26C &= 0xDFFFFCFF;
        }
    }
    if (D_8009D26C & 0x10000080) {
        D_8009D26C &= 0x7FFFFFFF;
    }
    if (D_8009D26C & 0x40000401) {
        D_8009D26C &= 0xEFFFFF7F;
        D_8009D26C &= 0x7FFFFFFF;
    }
    if (D_8009D26C & 0x20000300) {
        D_8009D26C &= 0xBFFFFBFE;
        D_8009D26C &= 0xEFFFFF7F;
        D_8009D26C &= 0x7FFFFFFF;
    }
    if ((D_800BE9A0 & 0xF000) == 0x7000) {
        D_8009D26C &= ~0x79;
        if (func_80062A34(1, 0) != 0) {
            x = D_800BE9A7;
            if (x < 0x14) {
                D_8009D26C |= 8;
            } else if (x >= 0xE7) {
                D_8009D26C |= 0x20;
            }
            x = D_800BE9A6;
            if (x < 0x14) {
                D_8009D26C |= 0x40;
            } else if (x >= 0xE7) {
                D_8009D26C |= 0x10;
            }
        } else {
            x = D_800BE9A7;
            if (x < 0x5A) {
                D_8009D26C |= 8;
                if (x < 0x14) {
                    D_8009D26C |= 1;
                }
            } else if (x >= 0xA1) {
                D_8009D26C |= 0x20;
                if (x >= 0xE7) {
                    D_8009D26C |= 1;
                }
            }
            x = D_800BE9A6;
            if (x < 0x5A) {
                D_8009D26C |= 0x40;
                if (x < 0x14) {
                    D_8009D26C |= 1;
                }
            } else if (x >= 0xA1) {
                D_8009D26C |= 0x10;
                if (x >= 0xE7) {
                    D_8009D26C |= 1;
                }
            }
        }
    }
    D_8009D1F4 = (D_8009D26C ^ D_8009D238) & D_8009D26C;
    D_8009D1E4 = (D_8009D26C ^ D_8009D238) & D_8009D238;
    } else {
        D_8009D26C = 0;
        D_8009D1F4 = 0;
        D_8009D1E4 = 0;
    }
}
