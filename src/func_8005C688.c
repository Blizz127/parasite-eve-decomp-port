typedef struct {
    unsigned char f0;
    unsigned char pad1[4];
    unsigned char f5;
    unsigned char f6;
    unsigned char pad7[25];
} Slot;

extern short *D_8009D048;
extern int D_8009D050;
extern unsigned int *D_8009D058;
extern int D_8009D064;
extern int D_8009D068;
extern int D_8009D0CC;
extern int D_8009D0D0;
extern int D_8009D040;
extern short D_800C0E48[];
extern unsigned int D_8009D05C[];
extern short D_800A1D9C[];
extern unsigned char D_800BEEAC[];
extern unsigned char D_8009DE64[];
extern Slot D_800C0EAC[];
extern int func_80052F70(void);
extern void func_800542A0(int);
extern unsigned char *func_8005DB44(int);

static inline unsigned char *rec(int a0)
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
        res = 0;
    done:
        return res;
    }
    return 0;
}

int func_8005C688(int min_b, int min_c)
{
    int i;
    int n;
    int k;
    int id;
    unsigned char *r;
    Slot *p;

    D_8009D048 = D_800C0E48;
    D_8009D050 = func_80052F70();
    D_8009D058 = D_8009D05C;
    D_8009D064 = 2;
    func_800542A0(D_8009D0CC != 0 ? 0x1FE : 0x200);
    for (i = 0; i < D_8009D064; i++) {
        D_8009D058[i] = 0;
    }
    n = 0;
    k = 0;
loop:
    if (k < D_8009D040) {
        if (k >= 0 && k < D_8009D040) {
            id = D_800A1D9C[k];
        } else {
            id = 0;
        }
        r = rec(id);
        if (r[7] + *(short *)(r + 0xE) < D_8009D0D0) {
            goto next;
        }
        if (r[8] + *(short *)(r + 0x10) < min_b) {
            goto next;
        }
        if (r[9] + *(short *)(r + 0x12) < min_c) {
            goto next;
        }
        if (r[4] == 0x93 || r[4] == 0x61) {
            goto next;
        }
        n++;
        D_8009D058[k >> 5] |= 1 << (k & 0x1F);
    next:
        k++;
        goto loop;
    }
    if (n != 0) {
        if (D_8009D0CC != 0) {
            for (p = D_800C0EAC; p < &D_800C0EAC[128] && !(p->f0 && p->f6 < 9 && (p->f5 & 0x10)); p++) {
            }
        } else {
            for (p = D_800C0EAC; p < &D_800C0EAC[128] && !(p->f0 && p->f6 == 9 && (p->f5 & 0x10)); p++) {
            }
        }
        if (p < &D_800C0EAC[128]) {
            n = -n;
        }
    }
    D_8009D068 = n != 0;
    return n;
}
