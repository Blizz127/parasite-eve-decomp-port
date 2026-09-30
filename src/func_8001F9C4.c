typedef struct Ent {
    unsigned char pad0[0xE];
    unsigned char fE;
    unsigned char padF[0x2A - 0xF];
    short f2A;
    unsigned char pad2C[2];
    short f2E;
    unsigned char pad30[2];
    short f32;
    unsigned char pad34[0x68 - 0x34];
    int f68;
    int f6C;
    int f70;
} Ent;

typedef struct Cfg {
    unsigned char pad0[4];
    unsigned int f4;
} Cfg;

typedef struct St {
    unsigned char pad0[4];
    short f4;
    unsigned char pad6[2];
    int f8;
    short fC;
    short fE;
    unsigned char pad10[2];
    unsigned char f12;
    unsigned char pad13[0x1C - 0x13];
    unsigned short f1C;
    unsigned char pad1E[0x28 - 0x1E];
    int f28;
    unsigned char pad2C[0x3C - 0x2C];
    unsigned short f3C;
    unsigned short f3E;
    unsigned short f40;
    unsigned short f42;
    unsigned short f44;
    unsigned short f46;
    unsigned char pad48[4];
    unsigned int f4C;
    unsigned char pad50[0x6C - 0x50];
    Cfg *f6C;
} St;

extern St *D_8009D278;
extern Ent *D_8009D254;
extern unsigned int D_8009D2E8;
extern short D_8009D228;
extern int D_8009CDDC;
extern unsigned char D_8009CE34;

extern void func_800201DC(void);
extern void func_8001A680(Ent *, unsigned short);
extern int func_80053E6C(int);
extern void func_80023E14(int);
extern void func_8005409C(int);
extern void func_8006F39C(int, Ent *);
extern void func_8006DE80(int, int, int, int, int);

void func_8001F9C4(void)
{
    St *st;
    unsigned int *fl;
    unsigned char used;

    int h;

    st = D_8009D278;
    fl = &st->f4C;
    if ((st->f4C & 3) == 1) {
        func_800201DC();
        if ((short)(D_8009D278->f40 -= D_8009D278->f3E) <= 0) {
            st->f4C &= ~3;
        }
    }
    if ((*fl & 0xC) == 4) {
        if ((short)(D_8009D278->f42 -= D_8009D278->f3C) <= 0) {
            *fl &= ~0xC;
            D_8009D2E8 &= ~0x10;
        }
    }
    if ((*fl & 0x30) == 0x10) {
        if ((short)(D_8009D278->f44 -= D_8009D278->f3C) <= 0) {
            *fl &= ~0x30;
        }
    }
    if ((*fl & 0xC0) == 0x40 || (*fl & 0xC0) == 0x80) {
        St *s = D_8009D278;

        if ((short)(s->f46 -= s->f3C) <= 0) {
            if ((*fl & 0xC0) == 0x80 && D_8009D254->fE == 0x11) {
                func_8001A680(D_8009D254, s->f12);
            }
            *fl &= ~0xC0;
        } else if ((*fl & 0xC0) == 0x80) {
            D_8009D254->f68 = 0;
            D_8009D254->f6C = 0;
            D_8009D254->f70 = 0;
        }
    }
    if (*fl & 0x100) {
        if (--D_8009D228 <= 0) {
            *fl &= ~0x100;
        }
    }
    if (*fl & 0x200) {
        if (D_8009D278->f8 <= 0x10000) {
            D_8009D278->f8 = 0x10000;
            *fl &= ~0x200;
        }
    }
    if (*fl & 0x400) {
        if (D_8009CDDC != 0) {
            if (D_8009D278->fC < (short)D_8009D278->f1C) {
                D_8009D278->fC++;
                D_8009D278->fE++;
            }
        }
        D_8009D278->f8 -= D_8009D278->f28 / (D_8009D278->f4 * 30);
        if (D_8009D278->f8 <= 0x10000) {
            D_8009D278->f8 = 0x10000;
            *fl &= ~0x400;
        }
    }
    if (*fl & 0x1000000) {
        if ((signed char)--D_8009CE34 <= 0) {
            *fl &= ~0x1000000;
        }
    }
    if (D_8009D278->f6C->f4 & 0x4000) {
        if (D_8009D278->fC > 0) {
            used = 0;
        loop:
            h = (short)D_8009D278->f1C;
            if (D_8009D278->fC < h / 5) {
                if (func_80053E6C(0xA) != 0) {
                    used = 1;
                    func_80023E14(0xA);
                    func_8005409C(0xA);
                    goto loop;
                }
                if (func_80053E6C(9) != 0) {
                    used = 1;
                    func_80023E14(9);
                    func_8005409C(9);
                    goto loop;
                }
                if (func_80053E6C(8) != 0) {
                    used = 1;
                    func_80023E14(8);
                    func_8005409C(8);
                    goto loop;
                }
                if (func_80053E6C(7) != 0) {
                    used = 1;
                    func_80023E14(7);
                    func_8005409C(7);
                    goto loop;
                }
                if (func_80053E6C(6) != 0) {
                    used = 1;
                    func_80023E14(6);
                    func_8005409C(6);
                    goto loop;
                }
            }
            if (used) {
                func_8006F39C(0x56, D_8009D254);
                func_8006DE80(0x4B4, 0, D_8009D254->f2A, D_8009D254->f2E, D_8009D254->f32);
            }
        }
    }
    if (D_8009D278->f6C->f4 & 0x8000) {
        if ((*fl & 3) == 1) {
            if (func_80053E6C(0xD) != 0) {
                func_80023E14(0xD);
                func_8005409C(0xD);
                func_8006F39C(0x57, D_8009D254);
                func_8006DE80(0x4B5, 0, D_8009D254->f2A, D_8009D254->f2E, D_8009D254->f32);
            } else if (func_80053E6C(0x11) != 0) {
                func_80023E14(0x11);
                func_8005409C(0x11);
                func_8006F39C(0x57, D_8009D254);
                func_8006DE80(0x4B5, 0, D_8009D254->f2A, D_8009D254->f2E, D_8009D254->f32);
            }
        }
        if ((*fl & 0xC) == 4) {
            if (func_80053E6C(0xF) != 0) {
                func_80023E14(0xF);
                func_8005409C(0xF);
                func_8006F39C(0x57, D_8009D254);
                func_8006DE80(0x4B5, 0, D_8009D254->f2A, D_8009D254->f2E, D_8009D254->f32);
            } else if (func_80053E6C(0x11) != 0) {
                func_80023E14(0x11);
                func_8005409C(0x11);
                func_8006F39C(0x57, D_8009D254);
                func_8006DE80(0x4B5, 0, D_8009D254->f2A, D_8009D254->f2E, D_8009D254->f32);
            }
        }
        if ((*fl & 0x30) == 0x10) {
            if (func_80053E6C(0xE) != 0) {
                func_80023E14(0xE);
                func_8005409C(0xE);
                func_8006F39C(0x57, D_8009D254);
                func_8006DE80(0x4B5, 0, D_8009D254->f2A, D_8009D254->f2E, D_8009D254->f32);
            } else if (func_80053E6C(0x11) != 0) {
                func_80023E14(0x11);
                func_8005409C(0x11);
                func_8006F39C(0x57, D_8009D254);
                func_8006DE80(0x4B5, 0, D_8009D254->f2A, D_8009D254->f2E, D_8009D254->f32);
            }
        }
        if ((*fl & 0xC0) == 0x40 || (*fl & 0xC0) == 0x80) {
            if (func_80053E6C(0x10) != 0) {
                func_80023E14(0x10);
                func_8005409C(0x10);
                func_8006F39C(0x57, D_8009D254);
                func_8006DE80(0x4B5, 0, D_8009D254->f2A, D_8009D254->f2E, D_8009D254->f32);
            } else if (func_80053E6C(0x11) != 0) {
                func_80023E14(0x11);
                func_8005409C(0x11);
                func_8006F39C(0x57, D_8009D254);
                func_8006DE80(0x4B5, 0, D_8009D254->f2A, D_8009D254->f2E, D_8009D254->f32);
            }
        }
        if (*fl & 0x1000) {
            if (func_80053E6C(0x11) != 0) {
                func_80023E14(0x11);
                func_8005409C(0x11);
                func_8006F39C(0x57, D_8009D254);
                func_8006DE80(0x4B5, 0, D_8009D254->f2A, D_8009D254->f2E, D_8009D254->f32);
            }
        }
    }
}
