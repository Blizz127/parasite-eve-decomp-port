typedef struct Item {
    unsigned char b0;
    unsigned char pad1[4];
    unsigned char b5;
    unsigned char b6;
    unsigned char b7;
    unsigned char pad8[0x18];
} Item;

typedef struct Pl {
    int w0;
    unsigned char pad4[2];
    unsigned short h06;
    unsigned short h08;
    unsigned char b0A;
    unsigned char padB;
    unsigned char b0C;
    unsigned char padD[0x13];
    signed char b20;
    unsigned char pad21;
    signed char b22;
    unsigned char pad23;
    int w24;
    unsigned short h28[7];
    unsigned char pad36[0xA];
    short h40;
    unsigned char pad42[6];
    short list[50];
    Item items[128];
    unsigned char pad10AC[0xD4];
    short stock[82];
} Pl;

extern Pl D_800C0E00;
extern short *D_8009D048;
extern int D_8009D050;
extern unsigned int *D_8009D058;
extern int D_8009D064;
extern unsigned int D_8009D05C[];
extern short D_800A1E6E[];
extern short D_800A1E8E[];
extern short D_800A1EAE[];
extern unsigned short *func_8005DB8C(int);
extern Item *func_8005DBAC(int);
extern int func_80052F70(void);
extern void func_80053D2C(int);
extern void func_800438C0(int);
extern void func_80042C78(void);

static inline int take_weapon(void)
{
    Item *p;
    short *q;
    int id;

    id = 0;
    for (p = D_800C0E00.items; p < D_800C0E00.items + 128; p++) {
        if (p->b0 != 0 && p->b6 != 9 && (p->b5 & 0x10)) {
            break;
        }
    }
    if (p < D_800C0E00.items + 128) {
        id = (p - D_800C0E00.items) + 256;
        for (q = D_800C0E00.stock; q < D_800C0E00.stock + 82; q++) {
            if (*q == id) {
                break;
            }
        }
        if (q < D_800C0E00.stock + 82) {
            *q = 0;
        }
    }
    return id;
}

static inline int take_armor(void)
{
    Item *p;
    short *q;
    int id;

    id = 0;
    for (p = D_800C0E00.items; p < D_800C0E00.items + 128; p++) {
        if (p->b0 != 0 && p->b6 == 9 && (p->b5 & 0x10)) {
            break;
        }
    }
    if (p < D_800C0E00.items + 128) {
        id = (p - D_800C0E00.items) + 256;
        for (q = D_800C0E00.stock; q < D_800C0E00.stock + 82; q++) {
            if (*q == id) {
                break;
            }
        }
        if (q < D_800C0E00.stock + 82) {
            *q = 0;
        }
    }
    return id;
}

void func_8005CCA4(void)
{
    int i;
    unsigned short *pp;
    int k;
    Item *t;
    int c;
    int id;

    pp = D_800C0E00.h28;
    for (i = 0; i < 7; i++) {
        *pp++ = *func_8005DB8C(i);
    }
    t = func_8005DBAC(0);
    D_800C0E00.h06 = D_800C0E00.h08 = *(unsigned short *)t;
    c = t->b7;
    D_800C0E00.w24 = 1;
    D_800A1EAE[0] = 0;
    D_800A1E8E[0] = 0;
    D_800A1E6E[0] = 0;
    D_8009D048 = D_800C0E00.list;
    D_800C0E00.b0C = c;
    D_8009D050 = func_80052F70();
    D_8009D058 = D_8009D05C;
    D_8009D064 = 2;
    k = t->b7;
    if (k >= 51) {
        k = 50;
    }
    D_800C0E00.b0C = k;
    if (D_8009D048 == D_800C0E00.list) {
        D_8009D050 = func_80052F70();
    }
    D_8009D048 = D_800C0E00.list;
    D_8009D050 = func_80052F70();
    D_8009D058 = D_8009D05C;
    D_8009D064 = 2;
    for (i = 49; i >= 0; i--) {
        D_8009D048[i] = 0;
    }
    func_80053D2C(68);
    func_80053D2C(150);
    func_80053D2C(63);
    func_80053D2C(1);
    func_80053D2C(6);
    i = take_weapon();
    if (i != 0) {
        func_80053D2C(i);
    }
    i = take_armor();
    if (i != 0) {
        func_80053D2C(i);
    }
    D_800C0E00.b22 = 1;
    D_800C0E00.b20 = 0;
    D_800C0E00.h40 = 61;
    func_800438C0(61);
    D_800C0E00.w0 = 0;
    D_800C0E00.b0A = 0;
    func_80042C78();
}
