typedef struct {
    short y;
    short x;
} Pt;

extern int D_8009D124;
extern int D_8009D128;
extern int D_8009D130;
extern Pt *D_8009D148;
extern unsigned int D_8009D14C;
extern unsigned int D_8009D150[4];
extern int D_800C0E44;
extern Pt D_800A22B0[12];
extern unsigned char D_800930A8[];
extern void func_800527C0(int);
extern void func_80061878(unsigned char *a0, int a1);
extern void func_80062090(int a0, int a1, int a2);

#define BLEND(c)                                                            \
    {                                                                       \
        int v_ = (c);                                              \
        int b_, t_, u_;                                                     \
        unsigned int *p_ = D_8009D150;                                      \
        D_8009D14C = v_ & 0xFFFFFF;                                         \
        b_ = (v_ >> 16) & 0xFF;                                             \
        t_ = (b_ + ((v_ >> 8) & 0xFF)) >> 1;                                \
        if (t_ >= 0x100) {                                                  \
            t_ = 0xFF;                                                      \
        }                                                                   \
        b_ = (b_ + (v_ & 0xFF)) >> 1;                                       \
        if (b_ < 0x100) {                                                   \
            t_ |= b_ << 8;                                                  \
        } else {                                                            \
            t_ |= 0xFF00;                                                   \
        }                                                                   \
        u_ = (((v_ >> 8) & 0xFF) + (v_ & 0xFF)) >> 1;                       \
        *p_ = t_ | ((u_ < 0x100) ? (u_ << 16) : 0xFF0000);                 \
    }

#define BLEND2(c)                                                            \
    {                                                                       \
        int v_ = (c);                                              \
        int b_, t_, u_;                                                     \
        unsigned int *p_ = D_8009D150;                                      \
        D_8009D14C = v_ & 0xFFFFFF;                                         \
        b_ = (v_ >> 16) & 0xFF;                                             \
        t_ = (b_ + ((v_ >> 8) & 0xFF)) >> 1;                                \
        if (t_ >= 0x100) {                                                  \
            t_ = 0xFF;                                                      \
        }                                                                   \
        u_ = (b_ + (v_ & 0xFF)) >> 1;                                       \
        if (u_ < 0x100) {                                                   \
            t_ |= u_ << 8;                                                  \
        } else {                                                            \
            t_ |= 0xFF00;                                                   \
        }                                                                   \
        u_ = (((v_ >> 8) & 0xFF) + (v_ & 0xFF)) >> 1;                       \
        *p_ = t_ | ((u_ < 0x100) ? (u_ << 16) : 0xFF0000);                 \
    }

#define PUSH(X, Y)                                                          \
    {                                                                       \
        int x_ = (X);                                                     \
        int y_ = (Y);                                                     \
        if (D_8009D148 < &D_800A22B0[12]) {                                 \
            D_8009D148->x = x_;                                             \
            (D_8009D148++)->y = y_;                                         \
        } else {                                                            \
            func_800527C0(4);                                               \
        }                                                                   \
    }

void func_800622BC(int w, int h, int a2, int a3)
{
    if (D_8009D130 != 0) {
        BLEND(D_8009D130);
    }
    D_8009D148 = D_800A22B0;
    PUSH(D_8009D124, D_8009D128);
    PUSH(D_8009D124 + w, D_8009D128);
    PUSH(D_8009D124, D_8009D128 + h);
    PUSH(D_8009D124 + w, D_8009D128 + h);
    func_80061878(D_800930A8, a2);
    D_8009D124 -= 2;
    D_8009D128 -= 2;
    func_80062090(w + 4, h + 4, a3);
    if (D_8009D130 != 0) {
        BLEND2(D_800C0E44);
    }
}
