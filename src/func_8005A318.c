/*
 * func_8005A318 - vram 0x8005A318, file 0x4AB18, size 0xCE4 (825 words), frame 0xA0.
 * Inventory transfer between the two item lists: move a slot byte (a1 >= 0)
 * or merge counts (a1 < 0, capped at 999), then consume a key/ammo item of
 * type 12/13 and finally drop the source item (a2 bit 0 clear).
 * era: cc1 2.7.2 -O2 -G8 + MASPSX_FORCE_ABSOLUTE_SYMBOLS=D_800C0E20,D_800C0E22.
 * Built from inline copies of the selector (func_80059F08, as a static inline so
 * `(who == 0) < 2` is not folded), the item lookups (func_800574A8 / func_8005332C /
 * func_8005415C shapes) and the item-remove body (func_80057D30); see
 * docs/evidence/func-8005A318/REPORT.md for the lever ladder.
 */
extern short *D_8009D048;
extern short *D_8009D04C;
extern int D_8009D050;
extern int D_8009D054;
extern unsigned char *D_8009D058;
extern int D_8009D064;
extern int D_8009D090[];
extern int D_8009D098[];
extern unsigned char D_8009D05C[];
extern unsigned char D_800A1F84[];
extern unsigned char D_800C0E48[];
extern unsigned char D_800BEEAC[];
extern unsigned char D_8009DE64[];
extern unsigned char D_800A1E64[];
extern unsigned char D_800A1E44[];
extern short D_800C1EB8[];
extern unsigned char D_800C0EAC[];
extern signed char D_800C0E20;
extern signed char D_800C0E22;
extern int func_80052F70(void);
extern unsigned char *func_8005DB44(int);
extern void func_8004CC50(int, int);
extern void func_80059A40(int *);
extern void func_80054E4C(int);
extern void func_80054CF8(void);
extern void func_800512AC(int, int);

#define CAP(p) ((((p)[9] + *(short *)((p) + 0x12)) > 999) ? 999 : ((p)[9] + *(short *)((p) + 0x12)))
#define HP(p) (*(unsigned short *)((p) + 0xA))
#define SH(p, o) (*(short *)((p) + (o)))
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
static inline int sel(unsigned int k)
{
    if (k < 2) {
        if (D_8009D098[k] != 0 && D_8009D04C != 0) {
            int n4 = D_8009D054;
            D_8009D048 = D_8009D04C;
            D_8009D058 = D_800A1F84;
            D_8009D064 = 4;
            D_8009D050 = n4;
        } else {
            D_8009D048 = (short *)D_800C0E48;
            D_8009D050 = func_80052F70();
            D_8009D058 = D_8009D05C;
            D_8009D064 = 2;
        }
        return D_8009D090[k];
    }
    return -1;
}

static inline unsigned char *lookup(int a0)
{
    int v;
    int w;
    unsigned char *res;

    if (a0 >= 0 && a0 < D_8009D050) {
        v = D_8009D048[a0];
        w = v;
        if ((unsigned int)(v - 0x100) < 0x80) {
            res = (unsigned char *)((v << 5) + (int)D_800BEEAC);
            goto done;
        }
        if ((unsigned int)(v - 1) < 0xFF) {
            res = func_8005DB44(v - 1);
            goto done;
        }
        if ((unsigned int)(w - 0x200) < 9) {
            res = (unsigned char *)((w << 5) + (int)D_8009DE64);
            goto done;
        }
    }
    res = 0;
done:
    return res;
}

static inline unsigned char gettype(int a0)
{
    int v;
    int w;
    unsigned char *p;

    if (a0 >= 0 && a0 < D_8009D050) {
        v = D_8009D048[a0];
        w = v;
        if ((unsigned int)(v - 0x100) < 0x80) {
            p = (unsigned char *)((v << 5) + (int)D_800BEEAC);
            goto got;
        }
        if ((unsigned int)(v - 1) < 0xFF) {
            p = func_8005DB44(v - 1);
            goto got;
        }
        if ((unsigned int)(w - 0x200) < 9) {
            p = (unsigned char *)((w << 5) + (int)D_8009DE64);
            goto got;
        }
    }
    p = 0;
got:
    return p != 0 ? p[6] : 0;
}

static inline unsigned char *typedesc(unsigned int t)
{
    unsigned char *q;

    if (t != 0 && t < 8) {
        q = D_800A1E64;
        if ((int)(t - 4) > 0) {
            q = D_800A1E64 + ((t - 5) << 5);
        }
    } else if (t >= 0x13) {
        q = D_800A1E64 + ((t - 0x13) << 5);
    } else {
        q = D_800A1E44;
    }
    return q;
}

#define CL(x) (((x) > 999) ? 999 : (x))

void func_8005A318(int who, int slot, int flags, int cnt)
{
    int tmp;
    int idx;
    int v;
    int w;
    int j;
    int k;
    int n;
    int c;
    int cls;
    int amt;
    unsigned int t;
    unsigned char *res;
    unsigned char *src;
    unsigned char *dst;
    unsigned char *q;
    short *e;
    short *sp;
    short *skip;
    short *car;
    int eight;

    idx = sel(who);
    src = 0;
    if (idx >= 0 && idx < D_8009D050) {
        int v, w;
        v = D_8009D048[idx];
        w = v;
        if ((unsigned int)(v - 0x100) < 0x80) {
            res = (unsigned char *)((v << 5) + (int)D_800BEEAC);
            goto l1_d;
        }
        if ((unsigned int)(v - 1) < 0xFF) {
            src = func_8005DB44(v - 1);
            goto l1_o;
        }
        if ((unsigned int)(w - 0x200) < 9) {
            res = (unsigned char *)((w << 5) + (int)D_8009DE64);
            goto l1_d;
        }
        res = 0;
    l1_d:
        src = res;
    }
l1_o:
    idx = sel(who == 0);
    dst = 0;
    if (idx >= 0 && idx < D_8009D050) {
        int v, w;
        v = D_8009D048[idx];
        w = v;
        if ((unsigned int)(v - 0x100) < 0x80) {
            res = (unsigned char *)((v << 5) + (int)D_800BEEAC);
            goto l2_d;
        }
        if ((unsigned int)(v - 1) < 0xFF) {
            dst = func_8005DB44(v - 1);
            goto l2_o;
        }
        if ((unsigned int)(w - 0x200) < 9) {
            res = (unsigned char *)((w << 5) + (int)D_8009DE64);
            goto l2_d;
        }
        res = 0;
    l2_d:
        dst = res;
    }
l2_o:
    if (slot >= 0) {
        c = (src + slot)[0x15];
        if ((c & 0xE0) != 0) {
            cls = c & 0xE0;
            for (k = 0; k < dst[0x14]; k++) {
                if (cls == ((dst + k)[0x15] & 0xE0)) {
                    break;
                }
            }
            if (k < dst[0x14]) {
                (dst + k)[0x15] = c;
                (src + slot)[0x15] = 0;
            } else {
                for (j = 0; j < dst[0x14]; j++) {
                    if (((dst + j)[0x15] & 0x1F) == 0) {
                        break;
                    }
                }
                if (j < dst[0x14]) {
                    (dst + j)[0x15] = (src + slot)[0x15];
                    (src + slot)[0x15] = 0;
                } else {
                    func_8004CC50(7, 0);
                }
            }
        } else {
            for (j = 0; j < dst[0x14]; j++) {
                if (((dst + j)[0x15] & 0x1F) == 0) {
                    break;
                }
            }
            if (j < dst[0x14]) {
                (dst + j)[0x15] = (src + slot)[0x15];
                (src + slot)[0x15] = 0;
            } else {
                func_8004CC50(7, 0);
            }
        }
    } else {
        SH(dst, 0xE) = CL(SH(src, 0xE) + SH(dst, 0xE));
        SH(dst, 0x10) = CL(SH(src, 0x10) + SH(dst, 0x10));
        SH(dst, 0x12) = CL(SH(src, 0x12) + SH(dst, 0x12));
        SH(src, 0xE) = 0;
        SH(src, 0x10) = 0;
        SH(src, 0x12) = 0;
        if (src[6] != 9 && src[9] < HP(src)) {
            amt = MIN(CAP(dst) - HP(dst), HP(src) - src[9]);
            HP(dst) += amt;
            {
            unsigned int t = src[6];
            if (t != 0 && t < 8) {
                q = D_800A1E64;
                if ((int)(t - 4) > 0) {
                    q = D_800A1E64 + ((t - 5) << 5);
                }
            } else if (t >= 0x13) {
                q = D_800A1E64 + ((t - 0x13) << 5);
            } else {
                q = D_800A1E44;
            }
            }
            HP(q) += (HP(src) - src[9]) - amt;
            if (CAP(q) < HP(q)) {
                HP(q) = CAP(q);
            }
            HP(src) = src[9];
        }
    }
    if (cnt < 999) {
        cnt = 12;
        if (flags & 1) {
            cnt = 13;
        }
        slot = 0;
        if (flags & 2) {
            for (; slot < 100; slot++) {
                if (D_800C1EB8[slot] != 0 && func_8005DB44(D_800C1EB8[slot] - 1)[6] == cnt) {
                    break;
                }
            }
            if (slot < 100) {
                D_800C1EB8[slot] = 0;
            }
        } else {
            D_8009D048 = (short *)D_800C0E48;
            D_8009D050 = func_80052F70();
            D_8009D058 = D_8009D05C;
            D_8009D064 = 2;
            if (D_8009D050 > 0) {
                do {
                    if (gettype(slot) == cnt) {
                        break;
                    }
                    slot++;
                } while (slot < D_8009D050);
                if (slot < D_8009D050) {
                    if (D_8009D048 == (short *)D_800C0E48 && D_800C0E22 == slot) {
                        func_80059A40(&tmp);
                    }
                    {
                        short *e;
                        int w, v;
                        e = (short *)((slot << 1) + (int)D_8009D048);
                        w = *e;
                        *e = 0;
                        v = w;
                        if (v >= 0x100) {
                            D_800C0EAC[(v - 0x100) << 5] = 0;
                        }
                    }
                    if (D_8009D048 == (short *)D_800C0E48 && D_800C0E22 == slot) {
                        D_800C0E22 = -1;
                        func_80054E4C(tmp);
                        func_80054CF8();
                        func_800512AC(3, 0);
                    }
                }
            }
        }
    }
    if (!(flags & 1)) {
        int unused[2];
        if ((unsigned int)who < 2) {
            if (D_8009D098[who] != 0 && D_8009D04C != 0) {
                int n4 = D_8009D054;
                D_8009D048 = D_8009D04C;
                D_8009D058 = D_800A1F84;
                D_8009D064 = 4;
                D_8009D050 = n4;
            } else {
                D_8009D048 = (short *)D_800C0E48;
                D_8009D050 = func_80052F70();
                D_8009D058 = D_8009D05C;
                D_8009D064 = 2;
            }
            slot = D_8009D090[who];
        } else {
            slot = -1;
        }
        {
        unsigned int t = src[6];
        if (t != 0 && t < 8) {
            q = D_800A1E64;
            if ((int)(t - 4) > 0) {
                q = D_800A1E64 + ((t - 5) << 5);
            }
        } else if (t >= 0x13) {
            q = D_800A1E64 + ((t - 0x13) << 5);
        } else {
            q = D_800A1E44;
        }
        }
        HP(q) += HP(src);
        if (CAP(q) < HP(q)) {
            HP(q) = CAP(q);
        }
        if (D_8009D048 == (short *)D_800C0E48 && D_800C0E22 == slot) {
            func_80059A40(&tmp);
        }
        {
            short *e;
            int w, v;
            e = (short *)((slot << 1) + (int)D_8009D048);
            w = *e;
            *e = 0;
            v = w;
            if (v >= 0x100) {
                D_800C0EAC[(v - 0x100) << 5] = 0;
            }
        }
        car = (short *)D_800C0E48;
        if (D_8009D048 == car && D_800C0E22 == slot) {
            D_800C0E22 = -1;
            func_80054E4C(tmp);
            func_80054CF8();
            func_800512AC(3, 0);
            asm volatile("" : "=r"(car) : "0"(car));
        }
        if (D_8009D048 == car) {
            if (D_800C0E20 == slot) {
                eight = 8;
                sp = D_8009D048;
                skip = sp - 1;
                for (; sp < D_8009D048 + D_8009D050; sp++) {
                    if (sp != skip) {
                        v = *sp;
                        w = v;
                        if ((unsigned int)(v - 0x100) < 0x80) {
                            res = (unsigned char *)((v << 5) + (int)D_800BEEAC);
                        } else if ((unsigned int)(v - 1) < 0xFF) {
                            res = func_8005DB44(v - 1);
                        } else if ((unsigned int)(w - 0x200) < 9) {
                            res = (unsigned char *)((w << 5) + (int)D_8009DE64);
                        } else {
                            res = 0;
                        }
                        if (res[6] == eight) {
                            break;
                        }
                    }
                }
                if (sp < D_8009D048 + D_8009D050) {
                    n = sp - D_8009D048;
                } else {
                    n = -1;
                }
                D_800C0E20 = n;
                func_800512AC(2, 0);
            } else if (D_800C0E22 == slot) {
                for (slot = 0; slot < D_8009D050; slot++) {
                    unsigned char *pp = lookup(slot);
                    if (pp != 0 && pp[6] == 9) {
                        break;
                    }
                }
                if (slot >= D_8009D050) {
                    slot = -1;
                }
                D_800C0E22 = slot;
                func_800512AC(3, 0);
            }
        }
    }
}
