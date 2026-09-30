typedef struct Inner {
    short f0;
    unsigned char pad2[4];
    short f6;
    unsigned char pad8[8];
    unsigned int f10;
} Inner;

typedef struct St {
    unsigned char pad0[4];
    short f4;
    unsigned char pad6[4];
    short fA;
    unsigned char padC[0x1E - 0xC];
    unsigned short f1E;
    unsigned char pad20[0x2A - 0x20];
    short f2A;
    unsigned char pad2C[0x4C - 0x2C];
    unsigned int f4C;
    unsigned char pad50[0x68 - 0x50];
    Inner *f68;
} St;

typedef struct Rec {
    unsigned int f0;
    unsigned char pad4;
    signed char f5;
    unsigned char f6;
    unsigned char pad7[9];
    int f10;
    unsigned char pad14[0x8C - 0x14];
    unsigned short f8C;
    unsigned char pad8E[0xCC - 0x8E];
    unsigned int fCC;
    unsigned char padD0[2];
} Rec;

typedef struct Ent {
    Rec *rec;
    unsigned char pad4[0x98 - 4];
    unsigned int f98;
} Ent;

extern St *D_8009D278;
extern signed char D_8009D2B0;

extern int func_80071A54(void);
extern void func_8001A680(void *, int);
extern void func_80028C48(Ent *);

void func_80028574(Ent *e) {
    register Rec *rec asm("$20");
    unsigned int *fw;
    unsigned int *pcc;
    unsigned int *fl;
    int amt;
    int base;
    unsigned char mode;
    unsigned int m;

    rec = e->rec;
    pcc = &rec->fCC;
    fw = &rec->f0;
    if (D_8009D278->f4C & 0x80000) {
        int t;

        t = D_8009D278->f1E - 25;
        base = (double)((t + D_8009D278->f4) * 6) * ((double)(D_8009D2B0 - 1) * 0.1 + 1.0) / 7.0;
    } else if (D_8009D278->f4C & 0x100000) {
        int q;

        q = (unsigned short)(D_8009D278->f1E / 5);
        q += D_8009D278->f68->f0;
        base = q * ((D_8009D278->f4 * 7 + 120) / 160) * D_8009D278->fA / D_8009D278->f2A + q;
    } else {
        unsigned char tbl[11] = { 0, 100, 60, 41, 0, 25, 0, 18, 0, 0, 13 };
        int q;

            q = (unsigned short)(D_8009D278->f1E / 5);
        q += D_8009D278->f68->f0;
        base = q * tbl[D_8009D278->f68->f10 & 0xF] / 100;
    }
    if (!(D_8009D278->f4C & 0x180000) && D_8009D278->f68->f6 != 8) {
        amt = base - rec->f8C / (int)(D_8009D278->f68->f10 & 0xF);
    } else {
        amt = base - rec->f8C;
    }
    if (amt > 0) {
            if (!(D_8009D278->f4C & 0x180000)) {
            fl = &D_8009D278->f68->f10;
            m = *fw & 0x38000;
            mode = 0;
            if (m == 0x8000) {
                amt = amt * 3 / 10;
            } else if (m == 0x18000) {
                amt = amt * 3 / 2;
            } else if (m == 0x20000) {
                amt = amt / 10;
            }
            switch (((*fw) >> 18) & 3) {
            case 2:
                break;
            case 3:
                *pcc |= 0x1000000;
                break;
            default:
                if (*fl & 0x400) {
                    switch (*pcc & 3) {
                    case 0:
                        if (func_80071A54() & 1) {
                            break;
                        }
                    case 2:
                        (*fw) |= 0x400;
                        break;
                    }
                }
                if (!((*fw) & 0x400)) {
                    if (*fl & 0x100) {
                        switch ((*pcc >> 14) & 3) {
                        case 1:
                            mode = 1;
                            break;
                        case 2:
                            mode = 2;
                            break;
                        }
                    }
                    if (mode != 2 && (*fl & 0x200)) {
                        switch (*(unsigned short *)((unsigned char *)pcc + 2) & 3) {
                        case 1:
                            mode = 1;
                            break;
                        case 2:
                            mode = 2;
                            break;
                        }
                    }
                    if (*fl & 0x800) {
                        switch ((*pcc >> 2) & 3) {
                        case 0:
                            if (func_80071A54() & 1) {
                                break;
                            }
                        case 2:
                            (*fw) = ((*fw) | 0x10) & ~0x3E0;
                            break;
                        }
                    }
                    if (*fl & 0x1000) {
                        switch ((*pcc >> 4) & 3) {
                        case 0:
                            if (func_80071A54() & 1) {
                                break;
                            }
                        case 2:
                            (*fw) |= 0x1800;
                            e->f98 |= 0x1000;
                            if (rec->f5) {
                                func_8001A680(e, (unsigned short)(signed char)rec->f6);
                            }
                            break;
                        }
                    }
                }
                break;
            }
            switch (mode) {
            case 1:
                amt = amt / 5;
                break;
            case 2:
                amt = amt * 3 / 2;
                break;
            }
        }
    }
    if (amt <= 0) {
        amt = 1;
        if ((*fw & 0x38000) == 0x18000) {
            amt = 2;
        }
        if (((*fw) & 0xC0000) == 0xC0000) {
            *pcc |= 0x1000000;
        }
    }
    rec->f10 -= amt;
    if (!(D_8009D278->f4C & 0x80000)) {
        func_80028C48(e);
    }
    (*fw) = ((*fw) & ~0x6000) | 0x4000;
}
