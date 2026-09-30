typedef struct {
    short h;
    short pad[3];
} VSlot;

typedef struct {
    unsigned char pad0[0x38];
    unsigned int f38;
    unsigned char pad1[0xF0 - 0x3C];
    unsigned int fF0;
    unsigned int fF4;
    unsigned char pad2[0x118 - 0xF8];
    unsigned short h118;
    unsigned short h11A;
} Voice;

extern unsigned char *D_8009D2C8;
extern unsigned int D_8009D2C4;
extern unsigned int D_8009D2B8;
extern VSlot D_800B002C[24];
extern void func_80088344(Voice *a0, unsigned int a1, unsigned int a2);
extern void func_800878F0();

void func_8008900C(Voice *a0, unsigned int a1, unsigned int a2, unsigned int *a3)
{
    unsigned int bit;
    unsigned int i;
    unsigned int j;
    unsigned int m;
    short *q;

    bit = 1;
    i = 0;
    m = a1 & *(unsigned int *)(D_8009D2C8 + 0x10);
    do {
        if (a1 & bit) {
            func_80088344(a0, bit, i);
            if (a0->fF4 != 0) {
                if (m & bit) {
                    if (a2 & bit) {
                        *a3 |= 1 << i;
                        a0->fF0 = i;
                        a0->fF4 |= 0x1FF93;
                    } else {
                        j = 0;
                        q = &D_800B002C[0].h;
                        while (j < 24) {
                            if (*q == 0) {
                                a0->fF4 |= 0x1FF93;
                                *a3 |= 1 << j;
                                a0->fF0 = j;
                                *q = 0x7FFF;
                                D_8009D2C4 |= 0x100;
                                j = 24;
                            } else {
                                j++;
                                q += 4;
                                if (j == 24) {
                                    a0->fF0 = j;
                                    *(unsigned int *)D_8009D2C8 |= 1;
                                }
                            }
                        }
                    }
                }
                if (D_8009D2B8 & bit) {
                    a0->h11A = 0;
                    a0->h118 = 0;
                }
                if (a0->fF0 < 24) {
                    func_800878F0(a0->fF0, &a0->fF0, a0->f38);
                }
            }
            a1 &= ~bit;
        }
        bit <<= 1;
        a0++;
        i++;
    } while (a1 != 0);
}
