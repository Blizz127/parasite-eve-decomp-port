typedef struct {
    unsigned char c[4][8];
    unsigned char pad[4];
} Pal;

typedef struct {
    unsigned char c[3];
    unsigned char pad[25];
} Pal2;

typedef struct { int f; } SW;
typedef struct { short f; } SH;
typedef struct { unsigned char f; } SB;
#define W(p, o) (((SW *)((unsigned char *)(p) + (o)))->f)
#define H(p, o) (((SH *)((unsigned char *)(p) + (o)))->f)
#define B(p, o) (((SB *)((unsigned char *)(p) + (o)))->f)

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
extern unsigned char D_800B692C[];
extern unsigned char D_800B692D[];
extern unsigned char D_800B692E[];

extern int D_8009CDDC;
extern int D_8009CDDC_b asm("D_8009CDDC");
extern int D_8009CDDC_c asm("D_8009CDDC");
extern int D_8009CDDC_d asm("D_8009CDDC");
extern unsigned int D_8009D250;
extern unsigned char *D_8009D278;
extern unsigned char *D_8009D254;
extern unsigned char *D_8009D20C;

extern void func_80032B0C(int, unsigned char *);
extern void func_80027A08(unsigned char *);

void func_8002D1F0(void)
{
    unsigned char buf[0x1A0];
    unsigned char *s1;
    unsigned char *s0;

    if (*(unsigned short *)(D_8009D278 + 0x10) >= 0x2328) {
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
        D_800B692E[D_8009CDDC * 28] = 0xF9;
        } else if ((D_8009D250 & 3) == 1) {
        D_800B00EC[D_8009CDDC * 36] = 0x50;
        D_800B00ED[D_8009CDDC * 36] = 0xA3;
        D_800B00EE[D_8009CDDC * 36] = 0xBE;
        D_800B00F4[D_8009CDDC * 36] = 0x50;
        D_800B00F5[D_8009CDDC * 36] = 0xA3;
        D_800B00F6[D_8009CDDC * 36] = 0xBE;
        D_800B00FC[D_8009CDDC * 36] = 0x50;
        D_800B00FD[D_8009CDDC * 36] = 0xA3;
        D_800B00FE[D_8009CDDC * 36] = 0xBE;
        D_800B0104[D_8009CDDC * 36] = 0x50;
        D_800B0105[D_8009CDDC * 36] = 0xA3;
        D_800B0106[D_8009CDDC * 36] = 0xBE;
        D_800B692C[D_8009CDDC * 28] = 0x50;
        asm("");
        D_800B692D[D_8009CDDC * 28] = 0xA3;
        D_800B692E[D_8009CDDC_b * 28] = 0xBE;
        } else if ((D_8009D250 & 3) == 2) {
        D_800B00EC[D_8009CDDC * 36] = 0x9F;
        D_800B00ED[D_8009CDDC * 36] = 0xFF;
        D_800B00EE[D_8009CDDC * 36] = 0xF9;
        D_800B00F4[D_8009CDDC * 36] = 0x0;
        D_800B00F5[D_8009CDDC * 36] = 0x46;
        D_800B00F6[D_8009CDDC * 36] = 0x82;
        D_800B00FC[D_8009CDDC * 36] = 0x9F;
        asm("");
        D_800B00FD[D_8009CDDC * 36] = 0xFF;
        D_800B00FE[D_8009CDDC * 36] = 0xF9;
        D_800B0104[D_8009CDDC * 36] = 0x0;
        D_800B0105[D_8009CDDC * 36] = 0x46;
        D_800B0106[D_8009CDDC * 36] = 0x82;
        D_800B692C[D_8009CDDC * 28] = 0x0;
        D_800B692D[D_8009CDDC * 28] = 0x46;
        D_800B692E[D_8009CDDC_c * 28] = 0x82;
        } else if ((D_8009D250 & 3) == 3) {
        D_800B00EC[D_8009CDDC * 36] = 0x50;
        D_800B00ED[D_8009CDDC * 36] = 0xA3;
        D_800B00EE[D_8009CDDC * 36] = 0xBE;
        D_800B00F4[D_8009CDDC * 36] = 0x50;
        D_800B00F5[D_8009CDDC * 36] = 0xA3;
        D_800B00F6[D_8009CDDC * 36] = 0xBE;
        D_800B00FC[D_8009CDDC * 36] = 0x50;
        D_800B00FD[D_8009CDDC * 36] = 0xA3;
        D_800B00FE[D_8009CDDC * 36] = 0xBE;
        D_800B0104[D_8009CDDC * 36] = 0x50;
        D_800B0105[D_8009CDDC * 36] = 0xA3;
        D_800B0106[D_8009CDDC * 36] = 0xBE;
        D_800B692C[D_8009CDDC * 28] = 0x50;
        asm("");
        D_800B692D[D_8009CDDC * 28] = 0xA3;
        D_800B692E[D_8009CDDC_d * 28] = 0xBE;
        }
    }
    if (B(D_8009D278, 0x56) != 0) {
        if (B(D_8009D278, 0x56) == 0x1E) {
            H(D_8009D278, 0x52) = H(D_8009D254, 0x210);
            H(D_8009D278, 0x54) = H(D_8009D254, 0x212);
        }
        func_80032B0C(0, D_8009D278 + 0x50);
        B(D_8009D278, 0x56)--;
    }
    if (B(D_8009D278, 0x5E) != 0) {
        if (B(D_8009D278, 0x5E) == 0x1E) {
            H(D_8009D278, 0x5A) = H(D_8009D254, 0x210);
            H(D_8009D278, 0x5C) = H(D_8009D254, 0x212);
            B(D_8009D278, 0x5F) = 1;
        }
        func_80032B0C(0, D_8009D278 + 0x58);
        B(D_8009D278, 0x5E)--;
    }
    for (s1 = D_8009D20C; s1 != 0; s1 = (unsigned char *)W(s1, 4)) {
        if (s1 == D_8009D254) {
            continue;
        }
        {
            unsigned char *t = (unsigned char *)W(s1, 0);
            if (t == 0) {
                continue;
            }
            s0 = t;
        }
        if (W(s0, 0) & 0x6000) {
            func_80027A08(s1);
            if ((W(s0, 0) & 0x6000) == 0x2000) {
                W(s0, 0) = (W(s0, 0) & ~0x6000) | 0x4000;
            }
        }
        if (B(s0, 0xD6) != 0) {
            if (B(s0, 0xD6) == 0x1E) {
                H(s0, 0xD2) = H(s1, 0x210);
                H(s0, 0xD4) = H(s1, 0x212);
            }
            func_80032B0C(1, s0 + 0xD0);
            B(s0, 0xD6)--;
        }
    }
}
