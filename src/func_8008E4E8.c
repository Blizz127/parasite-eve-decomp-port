extern unsigned char *D_8009D2C8;
extern unsigned char D_800B2900[];
extern void func_8008F0D0(unsigned char *a0, unsigned char *a1, unsigned int a2);

typedef struct { short a; short b; short c; } Pad3;

void func_8008E4E8(unsigned char *a0, unsigned int a1) {
    Pad3 unused;
    unsigned char *p;
    register unsigned int cnt asm("$4");
    register unsigned int i asm("$3");
    unsigned int adj;
    unsigned int c;

    p = *(unsigned char **)(a0 + 0x18);
    cnt = 1;
    adj = ((*(unsigned int *)D_8009D2C8 & 0x100) != 0) ? 0x30 : 0;
    if (p[8] < 0x80) {
        do {
            p += 8;
            cnt++;
        } while (p[8] < 0x80);
        p = *(unsigned char **)(a0 + 0x18);
    }
    i = 0;
    if (cnt == 0) {
        goto after;
    }
scan:
    if (p[2] >= a1) {
        goto after;
    }
    i++;
    p += 8;
    if (i < cnt) {
        goto scan;
    }
after:
    c = p[0];
    {
        unsigned int cur = *(unsigned short *)(a0 + 0x5A);
        if (c < 0x20) {
            if (cur == c) {
                return;
            }
        } else {
            if (cur == c + adj) {
                return;
            }
        }
    }
    {
        unsigned int d = p[8];
        unsigned int cur = *(unsigned short *)(a0 + 0x5A);
        if (d < 0x20) {
            if (cur == d) {
                return;
            }
        } else {
            if (cur == d + adj) {
                return;
            }
        }
    }
    {
        register unsigned int e asm("$2");
        register unsigned int v asm("$5");
        e = p[0];
        __asm__ __volatile__("" : "=r"(e) : "0"(e));
        v = e & 0xFF;
        if (e >= 0x20) {
            v += adj;
        }
        *(unsigned short *)(a0 + 0x5A) = v;
        v = v * 0x40;
        __asm__ __volatile__("" : "=r"(v) : "0"(v));
        {
            register unsigned char *base asm("$2");
            base = D_800B2900;
            __asm__ __volatile__("" : "=r"(base) : "0"(base));
            func_8008F0D0(a0, (unsigned char *)(v + (unsigned int)base),
                          *(unsigned int *)(D_800B2900 + v));
        }
        *(unsigned short *)(a0 + 0x10E) = p[3];
        *(unsigned short *)(a0 + 0x114) = p[4];
        *(unsigned int *)(a0 + 0x104) = p[5];
        {
            unsigned int t6 = p[6];
            *(unsigned int *)(a0 + 0xF4) |= 0x4000;
            *(unsigned short *)(a0 + 0x116) = t6;
        }
    }
}
