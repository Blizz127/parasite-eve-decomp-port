/* room_m0273i — func_80196348, blob offset 0x7360, 0x314 bytes.
 * Profile: -O2 -G0 -fno-schedule-insns. */

#define B(o, x) (*(unsigned char *)((char *)(o) + (x)))
#define H(o, x) (*(short *)((char *)(o) + (x)))
#define SH(o, x) (*(unsigned short *)((char *)(o) + (x)))
#define W(o, x) (*(int *)((char *)(o) + (x)))
#define P(o, x) (*(void **)((char *)(o) + (x)))

typedef struct {
    short s;
    short c;
} SC;

extern unsigned char *D_800F32D0;
extern unsigned short D_800942EC;
extern unsigned char D_8019AEF8;
extern short D_8019AEE8;
extern short D_8019AEB2;
extern short D_8019AEF0;
extern short D_8019AEF4;
extern short D_8019AEEC;
extern int D_8019AEB4;
extern unsigned char D_8019AE9C[];
extern unsigned short D_8019AEEA;
extern unsigned char D_8019AEFA;
extern short D_8019AEEE;
extern unsigned short D_8019AEF2;
extern short D_8019AEF6;
extern int *D_8009D254;
extern SC D_800966EC[];
extern int func_800DFF80();
extern int func_800DFFB8();

int func_80196348(int mode)
{
    volatile char frame_pad[8];
    unsigned char *state = P(D_800F32D0, 8);

    switch (mode) {
    case 0: {
        unsigned short cam = D_800942EC;

        D_8019AEF8 = 0;
        D_8019AEE8 = 0;
        D_8019AEB2 = 0;
        D_8019AEF0 = 0;
        D_8019AEF4 = 0;
        D_8019AEEC = cam - 512;
        D_8019AEB4 = W(state, 0x2C);
        return 0;
    }
    case 1:
        break;
    default:
        return 0;
    }
    {
        register int i asm("$3") = 0;
        register short *dst asm("$5") = (short *)D_8019AE9C;

        do {
            int *base = (int *)W(state, 0x238);
            int *src;

            if (i != 0) {
                src = (int *)((char *)base + 0x254);
            } else {
                src = (int *)((char *)base + 0x1B4);
            }
            dst[0] = src[0];
            dst[1] = src[1];
            dst[2] = src[2];
            i++;
            dst += 4;
        } while (i < 2);
    }
    {
        unsigned char *flag = P(P(state, 0), 0x18);
        unsigned short raw;
        register int timer asm("$3");
        int lim;

        if (*flag == 1) {
            *flag = 2;
        }
        raw = SH(state, 0x16);
        {
            register int eleven asm("$3") = 11;

            if (B(state, 0xE) != eleven) {
                return 0;
            }
        }
        timer = (int)raw << 16 >> 16;
        lim = (int)B(state, 0xF) - 1;
        if (timer < lim) {
            goto less;
        }
        {
            void *owner;

            D_8019AEF8 = 1;
            owner = P(state, 0);
            if (owner != 0) {
                B(P(owner, 0x18), 0) = 4;
            }
            return 1;
        }
    less:
        {
            register int below asm("$2") = timer < 34;

            if (!below) {
                unsigned short *p = &D_8019AEEA;
                unsigned short old = *p;

                *p = old - 1;
                if ((short)old > 0) {
                    W(state, 0x14) = 0x40000;
                }
            }
        }
    }
    if (D_8019AEFA != 0) {
        register int goal asm("$5") = D_8019AEF0;
        register int cur asm("$4") = D_8019AEEE;
        register int g2 asm("$3") = goal;
        register int n asm("$2");

        if (cur < goal) {
            n = g2 - D_8019AEF2;
        } else {
            register int cmp asm("$2");

            asm("slt %0, %1, %2" : "=r"(cmp) : "r"(goal), "r"(cur));
            if (cmp == 0) {
                goto angle;
            }
            n = g2 + D_8019AEF2;
        }
        D_8019AEF0 = n;
    angle:
        H(state, 0x3A) = (SH(state, 0x3A) + (unsigned short)D_8019AEF0) & 0xFFF;
        if (!(D_8019AEF4 < D_8019AEF6)) {
            D_8019AEEE = 0;
        }
    } else {
        int ang = func_800DFF80((char *)D_8009D254 + 0x1FC, (char *)state + 0x1FC);

        H(state, 0x3A) = func_800DFFB8(H(state, 0x3A), ((ang << 16) + 0x08000000) >> 16, 32);
    }
    {
        register int af asm("$4") = D_8019AEF4;

        if (af < 119) {
            register int mag asm("$3") = 0x22B63CBF;
            register int num asm("$2") = af << 13;
            register int hi asm("$7");
            register int q asm("$3");
            int sign;
            register int s asm("$2");
            register int t asm("$3");
            int r;
            register int base asm("$3");

            asm volatile("mult %0, %1" : : "r"(num), "r"(mag));
            sign = num >> 31;
            asm volatile("mfhi %0" : "=r"(hi));
            q = (hi >> 4) - sign;
            asm volatile("addiu %0, %1, -2048\n\tandi %0, %0, 4095" : "=r"(num) : "r"(q));
            s = D_800966EC[num].c;
            asm("nop\n\taddiu %0, %1, 4096" : "=r"(t) : "r"(s));
            r = -(t * 384) / 8192;
            base = D_8019AEB4;
            W(state, 0x2C) = (r << 16) + base;
        }
    }
    {
        register unsigned short *p asm("$3") = (unsigned short *)&D_8019AEF4;

        *p = *p + 1;
    }
    return 0;
}
