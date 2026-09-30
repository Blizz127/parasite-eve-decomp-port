extern unsigned char *D_8009D20C;
extern unsigned char *D_8009D254;
extern unsigned char *D_8009D278;
extern unsigned char D_8009D1CE;
extern unsigned char *D_8009D1F8;
extern unsigned char D_800915C0[];
extern unsigned char D_8009159C[];
extern int func_80071A54();
extern int func_8005BCB0();

signed char func_800255E4(void) {
    register unsigned char *e asm("$6");
    unsigned char *p;
    signed char r;
    short mx;
    signed char d;

    mx = 0;
    r = 0;
    e = D_8009D20C;
    if (e == 0) {
        goto after;
    }
    {
        unsigned char *self = D_8009D254;
        int mask = 0x40000;
loop:
        if (e == self) {
            goto next;
        }
        {
            unsigned char *o = *(unsigned char **)e;
            if (o == 0) {
                goto next;
            }
            if (*(int *)(o + 0x10) <= 0) {
                goto next;
            }
            {
                unsigned char c = o[4];
                if (mx < (signed char)c) {
                    mx = (signed char)c;
                }
            }
            if (*(int *)(o + 0xCC) & mask) {
                goto setm1;
            }
        }
next:
        e = *(unsigned char **)(e + 4);
        if (e != 0) {
            goto loop;
        }
    }
after:
    if (r != 0) {
        goto tail;
    }
    d = D_8009D278[4] - mx;
    if (d < 2) {
        goto small;
    }
    r = 0x50;
    goto have_r;
setm1:
    r = -1;
    goto after;
small:
    r = 0x28;
    if (d != 1) {
        if (d != 0) {
            r = 0xF;
            if (d == -1) {
                r = 0x19;
            }
        }
    }
have_r:
    p = D_8009D278;
    if (*(short *)(p + 0xC) * 10 < *(short *)(p + 0x1C)) {
        r = r * 3 / 2;
    }
    switch (((unsigned int)*(int *)(p + 0x4C) >> 25) & 7) {
    case 1: r = r * 3 / 2; break;
    case 2: r = r * 2; break;
    case 3: r = r * 3; break;
    case 4: r = r * 4; break;
    case 5: r = 0x64; break;
    }
    {
        int t = r;
        r = func_80071A54() % 100 < t;
    }
tail:
    if (r <= 0) {
    D_8009D1CE = 1;
    if (r == -1) {
        D_8009D1F8 = (unsigned char *)(func_8005BCB0() * 14 + (unsigned int)D_800915C0);
    } else {
        D_8009D1F8 = (unsigned char *)(func_8005BCB0() * 17 + (unsigned int)D_8009159C);
    }
    {
        unsigned char *q = D_8009D278;
        int f = *(int *)(q + 0x4C);
        int k = ((unsigned int)f >> 25) & 7;
        if (k < 5) {
            *(int *)(q + 0x4C) = (f & 0xF1FFFFFF) | (((k + 1) & 7) << 25);
        }
    }
    }
    return r;
}
