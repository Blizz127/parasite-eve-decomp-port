typedef struct Item {
    unsigned char pad0[4];
    unsigned char b4;
    unsigned char pad5[5];
    unsigned short hA;
    unsigned char padC[0x14];
} Item;

typedef struct Pl {
    unsigned char pad0[6];
    unsigned short h06;
    unsigned short h08;
    unsigned char padA;
    unsigned char b0B;
    unsigned char b0C;
    unsigned char padD[0x3B];
    short list[50];
    Item items[128];
    unsigned char pad10AC[0xC];
    short keys[100];
    short stock[82];
} Pl;

extern Pl D_800C0E00;
extern unsigned char D_800C0E0C;
extern short *D_8009D048;
extern int D_8009D050;
extern unsigned int *D_8009D058;
extern int D_8009D064;
extern unsigned int D_8009D05C[];
extern int D_8009D03C;
extern int D_8009D0CC;
extern int D_8009D0D0;
extern Item D_800A1E64[];
extern unsigned char D_800B0CE5[];
extern int func_80052F70(void);
extern int func_80051E58(void);
extern int func_8005C688(int);
extern void func_800515C0(int);
extern int func_800515F8(int *);
extern void func_80051684(int);
extern void func_80057D30(int);
extern void func_8005CAEC(void);
extern void func_8005DE88(void);
extern void func_80048918(int, int, int);
extern void func_8004C594(void);
extern void func_80042CC4(int);
extern void func_80042EDC(void);
extern void func_80042F20(void);
extern void func_80053128(void);
extern void func_8004D288(void);
extern void func_8005CCA4(void);
extern void func_8004BCE8(int);
extern void func_8005D020(void);

static inline void select_carried(void)
{
    D_8009D048 = D_800C0E00.list;
    D_8009D050 = func_80052F70();
    D_8009D058 = D_8009D05C;
    D_8009D064 = 2;
}

static inline int remove_key(int id)
{
    short *s;
    int n;

    for (s = D_800C0E00.keys; s < D_800C0E00.keys + 100; s++) {
        if (*s == id) {
            break;
        }
    }
    n = s >= D_800C0E00.keys + 100;
    if (!n) {
        *s = 0;
    }
    return n;
}

int func_8005D2B4(int cmd, int arg, int arg2_)
{
    register int arg2 asm("$5") = arg2_;
    short *p;
    short *q;
    short *r;
    int n;
    int k;
    unsigned short v;
    int tmp;

    switch (cmd) {
    case 1100:
        select_carried();
        arg = 0;
        for (p = D_8009D048; p < D_8009D048 + D_8009D050; p++) {
            arg += *p != 0;
        }
        return arg;
    case 1101:
        return (D_800C0E0C + func_80051E58() < 51) ? D_800C0E0C + func_80051E58() : 50;
    case 1102:
        k = D_8009D03C;
        arg2 = 0;
        if (arg >= k && arg < k + 3) {
            arg2 = D_800A1E64[arg - k].hA;
        } else {
            for (q = D_8009D048; q < D_8009D048 + D_8009D050; q++) {
                v = *q;
                arg2 += (((unsigned int)(v - 0x100) < 0x80) ? D_800C0E00.items[(short)v - 0x100].b4 : (short)v) == arg;
            }
        }
        return arg2;
    case 1103:
        D_8009D0CC = arg;
        D_8009D0D0 = arg2;
        return 0;
    case 1104:
        return func_8005C688(arg);
    case 1105:
        D_800C0E0C = (arg < 51) ? arg : 50;
        if (D_8009D048 == D_800C0E00.list) {
            D_8009D050 = func_80052F70();
        }
        return 0;
    case 1106:
        return D_800C0E00.h08;
    case 1107:
        return D_800C0E00.h06;
    case 1108:
        func_800515C0(arg);
        return 0;
    case 1109:
        return func_800515F8(0);
    case 1110:
        func_800515F8(&tmp);
        return tmp;
    case 1111:
        func_80051684(arg);
        return 0;
    case 1112:
        select_carried();
        for (r = D_8009D048; r < D_8009D048 + D_8009D050; r++) {
            if (*r == arg) {
                break;
            }
        }
        n = (r < D_8009D048 + D_8009D050) ? r - D_8009D048 : -1;
        if (n < 0) {
            return n;
        }
        func_80057D30(n);
        return 0;
    case 1113:
        func_8005CAEC();
        func_8005DE88();
        func_80048918(0, -3, -1);
        func_8004C594();
        return 0;
    case 1114:
        func_80042CC4(arg);
        return 0;
    case 1115:
        func_80042EDC();
        return 0;
    case 1116:
        func_80042F20();
        return 0;
    case 1117:
        func_80053128();
        D_800B0CE5[0] = 1;
        func_8004D288();
        D_800C0E00.b0B = (D_800C0E00.b0B + 1 >= 100) ? 99 : D_800C0E00.b0B + 1;
        func_8005CCA4();
        return 0;
    case 1118:
        func_8004BCE8(arg);
        return 0;
    case 1119:
        return remove_key(arg);
    case 1120:
        func_8005D020();
        return 0;
    }
    return 0;
}
