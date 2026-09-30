typedef struct Res {
    unsigned char pad0[4];
    unsigned int f4;
} Res;

typedef struct St {
    unsigned char pad0[0xC];
    unsigned short f0C;
    unsigned char pad0E[0x38 - 0xE];
    unsigned short f38;
    unsigned char f3A;
    unsigned char f3B;
    unsigned char pad3C[4];
    unsigned short f40;
    unsigned short f42;
    unsigned short f44;
    unsigned short f46;
    unsigned char pad48[4];
    union {
        unsigned int w;
        struct {
            unsigned int lo : 17;
            unsigned int elem : 2;
            unsigned int hi : 13;
        } b;
    } f4C;
    unsigned char pad50[0x6C - 0x50];
    Res *f6C;
} St;

typedef struct Atk {
    unsigned char f0;
    unsigned char f1;
} Atk;

typedef struct Ent {
    unsigned char pad0[0x18];
    Atk *f18;
    unsigned char pad1C[0x94 - 0x1C];
    unsigned char f94;
    unsigned char f95;
    unsigned char pad96[0xA0 - 0x96];
    short fA0;
    short fA2;
} Ent;

typedef struct StS {
    unsigned char pad0[0xC];
    short f0C;
} StS;

typedef struct Flags {
    unsigned int w;
} Flags;

/* signed view of the 16-bit field at +0xC */
#define SH(p, off) (*(short *)((unsigned char *)(p) + (off)))

extern St *D_8009D278;
extern unsigned char D_8009D1CE;
extern int D_8009D1F8;
extern short D_8009D228;
extern unsigned int D_8009D2E8;

extern int func_80071A54(void);
extern void func_8005409C(int);
extern int func_80054A88(int, int);
extern int func_8005485C(void);
extern void func_800553A4(short *, short *);

void func_80020288(Ent *e) {
    Flags *fp;
    unsigned int m;
    int r;
    int x;
    short xs;

    fp = (Flags *)&D_8009D278->f4C.w;
    switch (e->f18->f1) {
    case 1:
        if ((fp->w & 3) == 1) {
            return;
        }
        if (D_8009D278->f6C->f4 & 1) {
            if (func_80071A54() % 100 < 70) {
                return;
            }
        }
        if ((fp->w & 3) == 3) {
            fp->w &= ~3;
            return;
        }
        fp->w = (fp->w & ~3) | 1;
        D_8009D278->f40 = 9000;
        D_8009D278->f38 = e->f94;
        D_8009D278->f3A = 0;
        D_8009D278->f3B = e->f95;
        return;
    case 2:
        D_8009D278->f4C.w &= ~0x100;
        D_8009D228 = 0;
        D_8009D278->f4C.w &= ~0x200;
        D_8009D278->f4C.w &= ~0x400;
        D_8009D278->f4C.w &= ~0x800;
        return;
    case 3:
        if ((fp->w & 0xC) == 4) {
            return;
        }
        if (D_8009D278->f6C->f4 & 2) {
            if (func_80071A54() % 100 < 60) {
                return;
            }
        }
        if ((fp->w & 0xC) == 0xC) {
            fp->w &= ~0xC;
            return;
        }
        fp->w = (fp->w & ~0xC) | 4;
        D_8009D2E8 |= 0x10;
        r = func_80071A54();
        D_8009D278->f4C.b.elem = r % 4;
        D_8009D278->f42 = 9000;
        return;
    case 4:
        if ((fp->w & 0x30) == 0x10) {
            return;
        }
        if (D_8009D278->f6C->f4 & 4) {
            if (func_80071A54() % 100 < 60) {
                return;
            }
        }
        if ((fp->w & 0x30) == 0x30) {
            fp->w &= ~0x30;
            return;
        }
        fp->w = (fp->w & ~0x30) | 0x10;
        D_8009D278->f44 = 9000;
        return;
    case 5:
        if ((fp->w & 0xC0) == 0x80) {
            return;
        }
        if (D_8009D278->f6C->f4 & 8) {
            if (func_80071A54() % 100 < 50) {
                return;
            }
        }
        if ((fp->w & 0xC0) == 0xC0) {
            fp->w &= ~0xC0;
            return;
        }
        if (fp->w & 0x100) {
            fp->w &= ~0x100;
        }
        fp->w = (fp->w & ~0xC0) | 0x80;
        D_8009D278->f46 = 9000;
        return;
    case 6:
        m = fp->w & 0xC0;
        if (m == 0x80 || m == 0x40) {
            return;
        }
        if (D_8009D278->f6C->f4 & 8) {
            if (func_80071A54() % 100 < 70) {
                return;
            }
        }
        if ((fp->w & 0xC0) == 0xC0) {
            fp->w &= ~0xC0;
            return;
        }
        if (fp->w & 0x100) {
            fp->w &= ~0x100;
        }
        fp->w = (fp->w & ~0xC0) | 0x40;
        D_8009D278->f46 = 9000;
        return;
    case 7:
        fp->w |= 0x1000;
        return;
    case 8:
        if (fp->w & 0x200) {
            return;
        }
        if ((short)D_8009D278->f0C < 2) {
            return;
        }
        D_8009D278->f0C = (short)D_8009D278->f0C >> 1;
        return;
    case 9:
        x = ((StS *)D_8009D278)->f0C;
        D_8009D278->f4C.w &= ~0x100;
        D_8009D228 = 0;
        D_8009D278->f4C.w &= ~0x200;
        D_8009D278->f4C.w &= ~0x400;
        D_8009D278->f4C.w &= ~0x800;
        if (x >= 2) {
            ((StS *)D_8009D278)->f0C = 1;
        } else if (x == 1) {
            ((StS *)D_8009D278)->f0C = -1;
        }
        return;
    case 10:
    case 11:
        if (!(D_8009D278->f6C->f4 & 0x10)) {
            return;
        }
        if (func_80071A54() % 100 < 60) {
            return;
        }
        e->fA0 = func_8005485C();
        e->f18->f1 = 0;
        func_8005409C(e->fA0);
        D_8009D1CE = 1;
        D_8009D1F8 = func_80054A88(e->fA0, 0);
        return;
    case 12:
    case 13:
        if (!(D_8009D278->f6C->f4 & 0x10)) {
            return;
        }
        if (func_80071A54() % 100 < 60) {
            return;
        }
        func_800553A4(&e->fA0, &e->fA2);
        e->f18->f1 = 0;
        D_8009D1CE = 1;
        D_8009D1F8 = func_80054A88(e->fA0, 0);
        return;
    case 14:
    case 15:
        return;
    case 16:
        if (fp->w & 0x200) {
            return;
        }
        xs = SH(D_8009D278, 0xC);
        if (xs < 2) {
            return;
        }
        SH(D_8009D278, 0xC) = xs * 3 / 4;
        return;
    }
}
