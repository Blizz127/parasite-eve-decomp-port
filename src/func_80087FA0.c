extern unsigned int D_800BCD6C;
extern unsigned int D_800BCD74;
extern unsigned int D_8009D2C4;
extern void func_80089960(void);
extern void func_80089CF0(void);

#define U16(o) (*(unsigned short *)(v + (o)))
#define S16(o) (*(short *)(v + (o)))
#define I32(o) (*(int *)(v + (o)))
#define U32(o) (*(unsigned int *)(v + (o)))
#define PTR(o) (*(short **)(v + (o)))

void func_80087FA0(unsigned char *v, unsigned int bit)
{
    int n;
    unsigned int x;
    unsigned int m;
    short *t;
    int a;

    if (U16(0x72) != 0) {
        U16(0x72)--;
        n = I32(0x44) + I32(0x48);
        if ((n & 0xFFE00000) != (I32(0x44) & 0xFFE00000)) {
            U32(0xF4) |= 3;
        }
        I32(0x44) = n;
    }
    if (U16(0xBA) != 0) {
        if (--U16(0xBA) == 0) {
            D_800BCD6C ^= bit;
            D_8009D2C4 |= 0x10;
            func_80089960();
        }
    }
    if (U16(0xBC) != 0) {
        if (--U16(0xBC) == 0) {
            D_800BCD74 ^= bit;
            func_80089CF0();
        }
    }
    if (U16(0x96) != 0) {
        U16(0x96)--;
        x = U16(0x94) + U16(0x98);
        U16(0x94) = x;
        {
            unsigned int d = (x & 0x7F00) >> 8;
            if (x & 0x8000) {
                m = d * U32(0x30);
            } else {
                m = d * ((U32(0x30) * 15) >> 8);
            }
        }
        {
            unsigned int q = m >> 7;
            __asm__("" : "=r"(q) : "0"(q));
            U16(0x92) = q;
        }
        if (U16(0x8E) != 1) {
            t = PTR(0x1C);
            if (t[0] == 0 && t[1] == 0) {
                t += t[2];
            }
            n = (U16(0x92) * t[0]) >> 16;
            if (n != S16(0xE8)) {
                S16(0xE8) = n;
                U32(0xF4) |= 0x10;
                if (n >= 0) {
                    S16(0xE8) = n * 2;
                }
            }
        }
    }
    if (U16(0xA8) != 0) {
        U16(0xA8)--;
        U16(0xA6) += U16(0xAA);
        if (U16(0xA2) != 1) {
            t = PTR(0x20);
            if (t[0] == 0 && t[1] == 0) {
                t += t[2];
            }
            a = (S16(0x46) * (U16(0x6C) >> 8)) >> 7;
            n = (a * (U16(0xA6) >> 8)) << 9 >> 16;
            n = (n * t[0]) >> 15;
            if (n != S16(0xEA)) {
                S16(0xEA) = n;
                U32(0xF4) |= 3;
            }
        }
    }
    if (U16(0xB6) != 0) {
        U16(0xB6)--;
        U16(0xB4) += U16(0xB8);
        if (U16(0xB0) != 1) {
            t = PTR(0x24);
            if (t[0] == 0 && t[1] == 0) {
                t += t[2];
            }
            n = ((U16(0xB4) >> 8) * t[0]) >> 15;
            if (n != S16(0xEC)) {
                S16(0xEC) = n;
                U32(0xF4) |= 3;
            }
        }
    }
    if (U16(0x7A) != 0) {
        U16(0x7A)--;
        n = I32(0x34) + I32(0x4C);
        if ((n & 0xFFFF0000) != (I32(0x34) & 0xFFFF0000)) {
            U32(0xF4) |= 0x10;
        }
        I32(0x34) = n;
    }
}
