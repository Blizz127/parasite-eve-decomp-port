typedef struct Inner {
    unsigned char pad0[6];
    short f6;
    unsigned char pad8[8];
    unsigned int f10;
} Inner;

typedef struct St {
    unsigned char pad0[0xE];
    unsigned char f0E;
    unsigned char pad0F[1];
    short f10;
    unsigned char pad12[0x4C - 0x12];
    unsigned int f4C;
    unsigned char pad50[0x68 - 0x50];
    Inner *f68;
} St;

typedef struct Slot {
    unsigned short f0;
    unsigned char pad2[6];
} Slot;

typedef struct Entry {
    unsigned char pad0[12];
} Entry;

extern unsigned char D_8009CE3C;
extern unsigned int D_8009D1A0;
extern unsigned int D_8009D1F4;
extern signed char D_8009CE44;
extern unsigned char D_8009CE60;
extern unsigned char D_8009D1DC;
extern signed char D_8009D1F0;
extern St *D_8009D254;
extern St *D_8009D278;
extern unsigned char D_8009D288;
extern signed char D_8009D2B0;
extern signed char D_8009D2D8;
extern Entry D_8009E000[];
extern int D_800B0E08[];
extern Slot D_800BE834[];

extern void func_80021128(void);
extern void func_800258CC(int);
extern void func_80026600(Entry *);
extern int func_80026824(void);
extern void func_80026CF0(void);
extern void func_80026FD0(void);
extern void func_800275CC(Entry *, int);
extern void func_80051244(void);
extern void func_8005112C(void);
extern void func_8005C174(int);
extern int func_80062A34(int, int);
extern void func_80062F9C(void);
extern void func_80067CBC(void);
extern void func_8006DF50(int, int, int, int, int);

/* battle end-check: written out at each site (retail has three inlined copies; the
   first two cross-jump into one shared tail). */
#define END_CHECK()                                                        \
    ret = 0;                                                               \
    if ((unsigned short)(D_800BE834[0].f0 - 3) < 404) {                    \
        if (D_8009D254->f0E >= 4) {                                        \
            D_8009D1A0 |= 0x100;                                           \
        }                                                                  \
    }                                                                      \
    if (!((unsigned short)(D_800BE834[0].f0 - 387) < 21)) {                \
        n = D_8009CE3C;                                                    \
        for (i = 0; i < n; i++) {                                          \
            if ((unsigned short)(D_800BE834[i].f0 - 1) < 2) {              \
                func_80021128();                                           \
                break;                                                     \
            }                                                              \
        }                                                                  \
    }                                                                      \
    D_8009D288 = 0;                                                        \
    D_8009D278->f10 = 0;                                                   \
    D_8009D278->f4C |= 0x200000;                                           \
    func_80051244();

int func_80025EE8(void) {
    unsigned char ret;
    unsigned char f1;
    unsigned char f2;
    int k;
    signed char r;
    unsigned char i;
    unsigned char n;

    ret = 1;
    f2 = 0;
    f1 = 0;
    if (D_8009D2D8 == 0) {
        D_8009D1DC = 0;
        END_CHECK()
        func_800275CC(D_8009E000, D_8009CE44);
        if (D_800B0E08[0]) {
            func_8006DF50(D_800B0E08[0], 1100, 0, 128, 127);
        }
        return ret;
    }
    if ((D_8009D1F4 & 0x2000) && !func_80062A34(1, 0) && D_8009D1F0 != 2) {
        f1 = 1;
        if (D_8009D1DC == (D_8009D278->f68->f10 & 0xF)) {
            D_8009CE60 = ret;
        } else {
            D_8009CE60 = 0;
        }
        if (D_8009D1F0 == 1) {
            func_80062F9C();
        } else if (D_8009D1F0 == 2) {
            func_8005112C();
        }
        if (--D_8009D2D8 != 0) {
            D_8009D1DC = D_8009D278->f68->f10 & 0xF;
        } else {
            END_CHECK()
            func_800275CC(D_8009E000, D_8009CE44);
            if (D_800B0E08[0]) {
                func_8006DF50(D_800B0E08[0], 1100, 0, 128, 127);
            }
            return ret;
        }
        if (D_800B0E08[0]) {
            func_8006DF50(D_800B0E08[0], 1100, 0, 128, 127);
        }
    }
    k = D_8009D1F0;
    if (k == 0) {
        if (D_8009D2D8 != 0) {
            Inner *in = D_8009D278->f68;

            if (in->f6 == 8) {
                k = 3;
            } else {
                switch ((in->f10 >> 6) & 3) {
                case 1:
                    k = 1;
                    break;
                case 3:
                    k = 2;
                    break;
                default:
                    k = 0;
                    break;
                }
            }
            func_800258CC(k);
            if ((D_8009D1F4 & 0x200) && !f1) {
                D_8009CE60 = 0;
                if (D_8009D2B0) {
                    func_80026600(&D_8009E000[D_8009CE44]);
                    if (D_8009D1DC == 0) {
                        D_8009D2D8--;
                        D_8009D1DC = D_8009D278->f68->f10 & 0xF;
                    }
                }
                if (D_800B0E08[0]) {
                    func_8006DF50(D_800B0E08[0], 1100, 0, 128, 127);
                }
            }
        }
        if (D_8009D1DC == (D_8009D278->f68->f10 & 0xF) && (D_8009D1F4 & 0x80) && D_8009D2D8 != 0 && !f1) {
            D_8009D1F0 = 1;
            func_800275CC(D_8009E000, D_8009CE44);
            if (D_800B0E08[0]) {
                func_8006DF50(D_800B0E08[0], 1100, 0, 128, 127);
            }
            func_8005C174(1);
            f2 = 1;
            func_80067CBC();
        }
        if (D_8009D1F4 & 0x400) {
            if (!f2 && !f1) {
                func_800275CC(D_8009E000, D_8009CE44);
                if (D_8009CE3C) {
                    func_80026CF0();
                    func_80026FD0();
                } else {
                    ret = 0;
                }
                if (D_800B0E08[0]) {
                    func_8006DF50(D_800B0E08[0], 1101, 0, 128, 127);
                }
            }
        }
    } else {
        r = func_80026824();
        D_8009D1F0 = r;
        if (r == -1) {
            D_8009D1F0 = 0;
            END_CHECK()
            return ret;
        }
        if (r == 0) {
            func_80026FD0();
        }
    }
    return ret;
}
