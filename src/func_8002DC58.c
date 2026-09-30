typedef struct {
    unsigned char pad0[0x10];
    unsigned short f10;
    unsigned char f12;
    unsigned char pad13[0x39];
    unsigned int f4C;
} St;

typedef struct Ent {
    void *f0;
    struct Ent *next;
    unsigned char pad8[6];
    unsigned char fE;
    unsigned char fF;
    unsigned char pad10[0xA];
    unsigned short f1A;
    int f1C;
    unsigned char pad20[0x48];
    int f68;
    int f6C;
    int f70;
} Ent;

typedef struct {
    signed char pad0[5];
    signed char f5;
    signed char f6;
} Sub;

extern unsigned char D_800B00EC[];
extern unsigned char D_800B00ED[];
extern unsigned char D_800B00EE[];
extern unsigned char D_800B00F4[];
extern unsigned char D_800B00F5[];
extern unsigned char D_800B00F6[];
extern unsigned char D_800B00FC[];
extern unsigned char D_800B00FD[];
extern unsigned char D_800B00FE[];
extern unsigned char D_800B0104[];
extern unsigned char D_800B0105[];
extern unsigned char D_800B0106[];
extern unsigned char D_800B0110[];
extern unsigned char D_800B0111[];
extern unsigned char D_800B0112[];
extern unsigned char D_800B0118[];
extern unsigned char D_800B0119[];
extern unsigned char D_800B011A[];
extern unsigned char D_800B0120[];
extern unsigned char D_800B0121[];
extern unsigned char D_800B0122[];
extern unsigned char D_800B0128[];
extern unsigned char D_800B0129[];
extern unsigned char D_800B012A[];
extern unsigned char D_800B0134[];
extern unsigned char D_800B0135[];
extern unsigned char D_800B0136[];
extern unsigned char D_800B013C[];
extern unsigned char D_800B013D[];
extern unsigned char D_800B013E[];
extern unsigned char D_800B0144[];
extern unsigned char D_800B0145[];
extern unsigned char D_800B0146[];
extern unsigned char D_800B014C[];
extern unsigned char D_800B014D[];
extern unsigned char D_800B014E[];
extern unsigned char D_800B0158[];
extern unsigned char D_800B0159[];
extern unsigned char D_800B015A[];
extern unsigned char D_800B0160[];
extern unsigned char D_800B0161[];
extern unsigned char D_800B0162[];
extern unsigned char D_800B0168[];
extern unsigned char D_800B0169[];
extern unsigned char D_800B016A[];
extern unsigned char D_800B0170[];
extern unsigned char D_800B0171[];
extern unsigned char D_800B0172[];
extern unsigned char D_800B017C[];
extern unsigned char D_800B017D[];
extern unsigned char D_800B017E[];
extern unsigned char D_800B0184[];
extern unsigned char D_800B0185[];
extern unsigned char D_800B0186[];
extern unsigned char D_800B018C[];
extern unsigned char D_800B018D[];
extern unsigned char D_800B018E[];
extern unsigned char D_800B0194[];
extern unsigned char D_800B0195[];
extern unsigned char D_800B0196[];
extern unsigned char D_800B01A0[];
extern unsigned char D_800B01A1[];
extern unsigned char D_800B01A2[];
extern unsigned char D_800B01A8[];
extern unsigned char D_800B01A9[];
extern unsigned char D_800B01AA[];
extern unsigned char D_800B01B0[];
extern unsigned char D_800B01B1[];
extern unsigned char D_800B01B2[];
extern unsigned char D_800B01B8[];
extern unsigned char D_800B01B9[];
extern unsigned char D_800B01BA[];
extern unsigned char D_800B692C[];
extern unsigned char D_800B692D[];
extern unsigned char D_800B692E[];
extern unsigned char D_800B6948[];
extern unsigned char D_800B6949[];
extern unsigned char D_800B694A[];

extern int D_8009CDDC;
extern int D_8009CDDC_a asm("D_8009CDDC");
extern int D_8009CDDC_b asm("D_8009CDDC");
extern int D_8009CDDC_c asm("D_8009CDDC");
extern int D_8009CDDC_d asm("D_8009CDDC");
extern unsigned int D_8009D250;
extern St *D_8009D278;
extern Ent *D_8009D254;
extern Ent *D_8009D20C;
extern unsigned char D_8009CE74;
extern unsigned int D_8009D2E8;
extern unsigned int D_8009D1A0;
extern int D_8009D28C;

extern void func_8001A680(Ent *, int);
extern unsigned char func_80027A08(Ent *);
extern void func_800866A4(int, int);
extern void func_800703F4(void);
extern int func_8006D60C(int);
extern void func_8002F9CC(void);
extern void func_800295E4(void);
extern void func_800293F4(int);

void func_8002DC58(void)
{
    char pad[0x300];
    unsigned char s;
    Ent *e;

    s = 1;
    D_8009D254->f68 = 0;
    D_8009D254->f6C = 0;
    D_8009D254->f70 = 0;
    if (D_8009D278->f10 >= 9000) {
        if ((D_8009D250 & 3) == 0) {
            D_800B00EC[D_8009CDDC * 36] = 0x0;
            D_800B00ED[D_8009CDDC * 36] = 0x46;
            D_800B00EE[D_8009CDDC * 36] = 0x82;
            D_800B00F4[D_8009CDDC * 36] = 0x9F;
            D_800B00F5[D_8009CDDC * 36] = 0xFF;
            D_800B00F6[D_8009CDDC * 36] = 0xF9;
            D_800B00FC[D_8009CDDC * 36] = 0x0;
            D_800B00FD[D_8009CDDC * 36] = 0x46;
            D_800B00FE[D_8009CDDC * 36] = 0x82;
            D_800B0104[D_8009CDDC * 36] = 0x9F;
            D_800B0105[D_8009CDDC * 36] = 0xFF;
            D_800B0106[D_8009CDDC * 36] = 0xF9;
            D_800B692C[D_8009CDDC * 28] = 0x9F;
            D_800B692D[D_8009CDDC * 28] = 0xFF;
            D_800B692E[D_8009CDDC_a * 28] = 0xF9;
        } else if ((D_8009D250 & 3) == 1) {
            register int ka asm("$5") = 0x50; register int kb asm("$6") = 0xA3; register int kc asm("$4") = 0xBE;
            D_800B00EC[D_8009CDDC * 36] = ka;
            D_800B00ED[D_8009CDDC * 36] = kb;
            D_800B00EE[D_8009CDDC * 36] = kc;
            D_800B00F4[D_8009CDDC * 36] = ka;
            D_800B00F5[D_8009CDDC * 36] = kb;
            D_800B00F6[D_8009CDDC * 36] = kc;
            D_800B00FC[D_8009CDDC * 36] = ka;
            D_800B00FD[D_8009CDDC * 36] = kb;
            D_800B00FE[D_8009CDDC * 36] = kc;
            D_800B0104[D_8009CDDC * 36] = ka;
            D_800B0105[D_8009CDDC * 36] = kb;
            D_800B0106[D_8009CDDC * 36] = kc;
            D_800B692C[D_8009CDDC * 28] = ka;
            D_800B692D[D_8009CDDC * 28] = kb;
            D_800B692E[D_8009CDDC_b * 28] = kc;
        } else if ((D_8009D250 & 3) == 2) {
            register int k9 asm("$7") = 0x9F; register int kf asm("$8") = 0xFF;
            D_800B00EC[D_8009CDDC * 36] = k9;
            D_800B00ED[D_8009CDDC * 36] = kf;
            D_800B00EE[D_8009CDDC * 36] = 0xF9;
            D_800B00F4[D_8009CDDC * 36] = 0x0;
            D_800B00F5[D_8009CDDC * 36] = 0x46;
            D_800B00F6[D_8009CDDC * 36] = 0x82;
            D_800B00FC[D_8009CDDC * 36] = k9;
            D_800B00FD[D_8009CDDC * 36] = kf;
            D_800B00FE[D_8009CDDC * 36] = 0xF9;
            D_800B0104[D_8009CDDC * 36] = 0x0;
            D_800B0105[D_8009CDDC * 36] = 0x46;
            D_800B0106[D_8009CDDC * 36] = 0x82;
            D_800B692C[D_8009CDDC * 28] = 0x0;
            D_800B692D[D_8009CDDC * 28] = 0x46;
            D_800B692E[D_8009CDDC_c * 28] = 0x82;
        } else if ((D_8009D250 & 3) == 3) {
            register int ka asm("$5") = 0x50; register int kb asm("$6") = 0xA3; register int kc asm("$4") = 0xBE;
            D_800B00EC[D_8009CDDC * 36] = ka;
            D_800B00ED[D_8009CDDC * 36] = kb;
            D_800B00EE[D_8009CDDC * 36] = kc;
            D_800B00F4[D_8009CDDC * 36] = ka;
            D_800B00F5[D_8009CDDC * 36] = kb;
            D_800B00F6[D_8009CDDC * 36] = kc;
            D_800B00FC[D_8009CDDC * 36] = ka;
            D_800B00FD[D_8009CDDC * 36] = kb;
            D_800B00FE[D_8009CDDC * 36] = kc;
            D_800B0104[D_8009CDDC * 36] = ka;
            D_800B0105[D_8009CDDC * 36] = kb;
            D_800B0106[D_8009CDDC * 36] = kc;
            D_800B692C[D_8009CDDC * 28] = ka;
            D_800B692D[D_8009CDDC * 28] = kb;
            D_800B692E[D_8009CDDC_d * 28] = kc;
        }
    }
    if (D_8009D278->f4C & 0x2000) {
        if ((D_8009D250 & 3) == 0) {
            D_800B0158[D_8009CDDC * 72] = 0xFF;
            D_800B0159[D_8009CDDC * 72] = 0x3D;
            D_800B015A[D_8009CDDC * 72] = 0x81;
            D_800B0160[D_8009CDDC * 72] = 0x83;
            D_800B0161[D_8009CDDC * 72] = 0x13;
            D_800B0162[D_8009CDDC * 72] = 0x1;
            D_800B0168[D_8009CDDC * 72] = 0xFF;
            D_800B0169[D_8009CDDC * 72] = 0x3D;
            D_800B016A[D_8009CDDC * 72] = 0x81;
            D_800B0170[D_8009CDDC * 72] = 0x83;
            D_800B0171[D_8009CDDC * 72] = 0x13;
            D_800B0172[D_8009CDDC_a * 72] = 0x1;
        } else if ((D_8009D250 & 3) == 1) {
            D_800B0158[D_8009CDDC * 72] = 0xC1;
            D_800B0159[D_8009CDDC * 72] = 0x28;
            D_800B015A[D_8009CDDC * 72] = 0x41;
            D_800B0160[D_8009CDDC * 72] = 0xC1;
            D_800B0161[D_8009CDDC * 72] = 0x28;
            D_800B0162[D_8009CDDC * 72] = 0x41;
            D_800B0168[D_8009CDDC * 72] = 0xC1;
            D_800B0169[D_8009CDDC * 72] = 0x28;
            D_800B016A[D_8009CDDC * 72] = 0x41;
            D_800B0170[D_8009CDDC * 72] = 0xC1;
            D_800B0171[D_8009CDDC * 72] = 0x28;
            D_800B0172[D_8009CDDC_b * 72] = 0x41;
        } else if ((D_8009D250 & 3) == 2) {
            D_800B0158[D_8009CDDC * 72] = 0x83;
            D_800B0159[D_8009CDDC * 72] = 0x13;
            D_800B015A[D_8009CDDC * 72] = 0x1;
            D_800B0160[D_8009CDDC * 72] = 0xFF;
            D_800B0161[D_8009CDDC * 72] = 0x3D;
            D_800B0162[D_8009CDDC * 72] = 0x81;
            D_800B0168[D_8009CDDC * 72] = 0x83;
            D_800B0169[D_8009CDDC * 72] = 0x13;
            D_800B016A[D_8009CDDC * 72] = 0x1;
            D_800B0170[D_8009CDDC * 72] = 0xFF;
            D_800B0171[D_8009CDDC * 72] = 0x3D;
            D_800B0172[D_8009CDDC_c * 72] = 0x81;
        } else if ((D_8009D250 & 3) == 3) {
            D_800B0158[D_8009CDDC * 72] = 0xC1;
            D_800B0159[D_8009CDDC * 72] = 0x28;
            D_800B015A[D_8009CDDC * 72] = 0x41;
            D_800B0160[D_8009CDDC * 72] = 0xC1;
            D_800B0161[D_8009CDDC * 72] = 0x28;
            D_800B0162[D_8009CDDC * 72] = 0x41;
            D_800B0168[D_8009CDDC * 72] = 0xC1;
            D_800B0169[D_8009CDDC * 72] = 0x28;
            D_800B016A[D_8009CDDC * 72] = 0x41;
            D_800B0170[D_8009CDDC * 72] = 0xC1;
            D_800B0171[D_8009CDDC * 72] = 0x28;
            D_800B0172[D_8009CDDC_d * 72] = 0x41;
        }
    }
    if (D_8009D254->fE != D_8009D278->f12) {
        func_8001A680(D_8009D254, D_8009D278->f12);
    }
    switch (D_8009CE74) {
    case 0:
        D_8009D2E8 |= 1;
        for (e = D_8009D20C; e != 0; e = e->next) {
            if (e == D_8009D254) {
                continue;
            }
            if (e->f0 == 0 || ((Sub *)e->f0)->f5 == 1) {
                continue;
            }
            if ((unsigned int)(e->fE - 2) >= 2) {
                if (e->fF == e->f1A) {
                    func_8001A680(e, (unsigned short)((Sub *)e->f0)->f6);
                } else if (e->f1C != 0x10000) {
                    e->f1C = 0x10000;
                }
            }
            if (!func_80027A08(e)) {
                s = 0;
            }
        }
        if (s) {
            func_800866A4(0, 0xFF);
            func_800703F4();
            D_8009CE74++;
        }
        break;
    case 1:
        if (func_8006D60C(0) == 1) {
            break;
        }
        func_8002F9CC();
        if (!(D_8009D1A0 & 0x1800)) {
            func_8001A680(D_8009D254, 0x15);
        } else {
            D_8009D1A0 &= ~0x1800;
            func_8001A680(D_8009D254, 0x18);
        }
        func_800295E4();
        D_800B00EC[0] = 0; D_800B00ED[0] = 0x46; D_800B00EE[0] = 0x82;
        D_800B00F4[0] = 0x9F; D_800B00F5[0] = 0xFF; D_800B00F6[0] = 0xF9;
        D_800B00FC[0] = 0; D_800B00FD[0] = 0x46; D_800B00FE[0] = 0x82;
        D_800B0104[0] = 0x9F; D_800B0105[0] = 0xFF; D_800B0106[0] = 0xF9;
        D_800B692C[0] = 0x9F; D_800B692D[0] = 0xFF; D_800B692E[0] = 0xF9;
        D_800B0110[0] = 0; D_800B0111[0] = 0x46; D_800B0112[0] = 0x82;
        D_800B0118[0] = 0x9F; D_800B0119[0] = 0xFF; D_800B011A[0] = 0xF9;
        D_800B0120[0] = 0; D_800B0121[0] = 0x46; D_800B0122[0] = 0x82;
        D_800B0128[0] = 0x9F; D_800B0129[0] = 0xFF; D_800B012A[0] = 0xF9;
        D_800B6948[0] = 0x9F; D_800B6949[0] = 0xFF; D_800B694A[0] = 0xF9;
        D_800B0134[0] = 0; D_800B0135[0] = 0x82; D_800B0136[0] = 0x36;
        D_800B013C[0] = 0x4A; D_800B013D[0] = 0xFF; D_800B013E[0] = 0x3B;
        D_800B0144[0] = 0; D_800B0145[0] = 0x82; D_800B0146[0] = 0x36;
        D_800B014C[0] = 0x4A; D_800B014D[0] = 0xFF; D_800B014E[0] = 0x3B;
        D_800B017C[0] = 0; D_800B017D[0] = 0x82; D_800B017E[0] = 0x36;
        D_800B0184[0] = 0x4A; D_800B0185[0] = 0xFF; D_800B0186[0] = 0x3B;
        D_800B018C[0] = 0; D_800B018D[0] = 0x82; D_800B018E[0] = 0x36;
        D_800B0194[0] = 0x4A; D_800B0195[0] = 0xFF; D_800B0196[0] = 0x3B;
        D_800B0158[0] = 0xFF; D_800B0159[0] = 0x3D; D_800B015A[0] = 0x81;
        D_800B0160[0] = 0x83; D_800B0161[0] = 0x13; D_800B0162[0] = 1;
        D_800B0168[0] = 0xFF; D_800B0169[0] = 0x3D; D_800B016A[0] = 0x81;
        D_800B0170[0] = 0x83; D_800B0171[0] = 0x13; D_800B0172[0] = 1;
        D_800B01A0[0] = 0xFF; D_800B01A1[0] = 0x3D; D_800B01A2[0] = 0x81;
        D_800B01A8[0] = 0x83; D_800B01A9[0] = 0x13; D_800B01AA[0] = 1;
        D_800B01B0[0] = 0xFF; D_800B01B1[0] = 0x3D; D_800B01B2[0] = 0x81;
        D_800B01B8[0] = 0x83; D_800B01B9[0] = 0x13; D_800B01BA[0] = 1;
        D_8009D28C = 12;
        func_800293F4(0);
        break;
    }
}
