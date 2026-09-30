typedef struct Blk6 {
    int w[6];
} Blk6;

typedef struct Blk2 {
    int w[2];
} Blk2;

typedef struct Stat {
    unsigned char pad0[0xC];
    unsigned int lo : 10;
    unsigned int mid : 10;
    unsigned int grade : 2;
    unsigned int hi : 10;
} Stat;

typedef struct Ent {
    unsigned char pad0[0x68];
    Stat *f68;
    Blk2 *f6C;
} Ent;

typedef struct Item {
    unsigned char pad0[6];
    unsigned char b6;
    unsigned char pad7[3];
    unsigned short h0A;
} Item;

extern Ent **D_8009D254;
extern int D_8009D010;
extern int D_8009D018;
extern void *D_8009D1E0;
extern int func_8005B89C(void);
extern void func_800254BC(int);
extern void func_80051980(int, void *);
extern void func_80051E64(void *);
extern void func_80052E30(int);
extern void func_80051CC4(void);
extern void func_80062F9C(void);

static inline int item_grade(int *arg)
{
    int k;

    if (((Item *)*arg)->b6 != 0 && ((Item *)*arg)->b6 < 8) {
        k = ((Item *)*arg)->b6 - 4;
        if (k <= 0) {
            k = 1;
        }
    } else {
        k = (((Item *)*arg)->b6 >= 19) ? ((Item *)*arg)->b6 - 18 : 0;
    }
    return k;
}

void func_800512AC(int cmd, int *arg)
{
    Ent *e;

    e = (D_8009D254 != 0 && *D_8009D254 != 0) ? *D_8009D254 : 0;
    switch (cmd) {
    case 0:
        D_8009D010 = *arg + 3;
        break;
    case 1:
        D_8009D010 = *arg + 387;
        break;
    case 2:
        if (e == 0) {
            break;
        }
        if (D_8009D1E0 != 0) {
            *(Blk6 *)D_8009D1E0 = *(Blk6 *)e->f68;
            func_80051980(0, D_8009D1E0);
        }
        if (func_8005B89C() != 0) {
            D_8009D010 = 407;
        } else {
            func_800254BC(407);
        }
        break;
    case 3:
        if (e == 0) {
            break;
        }
        if (D_8009D1E0 != 0) {
            *(Blk2 *)D_8009D1E0 = *e->f6C;
            func_80051E64(D_8009D1E0);
        }
        if (func_8005B89C() != 0) {
            D_8009D010 = 408;
        } else {
            func_800254BC(408);
        }
        break;
    case 5:
        if (e == 0) {
            break;
        }
        e->f68->lo = ((Item *)*arg)->h0A;
        {
            Stat *s = e->f68;
            s->grade = item_grade(arg);
        }
        break;
    case 8:
        D_8009D010 = 409;
        break;
    case 9:
        D_8009D010 = -1;
        break;
    case 10:
        D_8009D010 = 1000;
        break;
    case 11:
        D_8009D010 = 1;
        break;
    case 12:
        D_8009D018 = 4;
        func_80052E30(0);
        func_80051CC4();
        D_8009D010 = 2;
        break;
    }
    if (D_8009D010 != 0) {
        func_80062F9C();
    }
}
