/*
 * func_8006BECC - vram 0x8006BECC, size 0x300. CD room-data load state machine (states 0..6 via
 * jtbl_800113B0): read area header (func_8006E6A8/func_8006E7E8 poll), relocate its entries
 * (func_8006E1C0), read the second block, fix up the per-object pointer table at +0x1C0.
 * era: cc1 2.7.2 -O2 -G0 + MASPSX_DISPATCH_FOLD=jtbl_800113B0 + MASPSX_THREE_WORD_SYMBOL_STORE=1.
 * Levers: goto top (no loop.c loop, so constants are rematerialized like retail); separate
 * D_800930D8/D_800930DA symbols with an explicit k = area + 3 / area + 8 index; loop 1 with an
 * indexed q + i*0x14 argument; loop 2 via a copy e = q with indexed e[i*0xC+..] accesses.
 * NOTE: `int pad[4]` is an unused local. Retail reserves 32 bytes of locals (.frame vars=32)
 * that are never stored; the two variable-bound for loops account for 16 on 2.7.2, the other
 * 16 come from a 16-byte unused local. The retail frame 0x50 is the evidence.
 */
extern unsigned char D_800B0CD8[];
extern unsigned char D_800B0CE2;
extern int D_800B0DD8;
extern unsigned short D_800930D8[];
extern unsigned short D_800930DA[];
extern unsigned char D_8009D25C;
extern int func_8006E6A8(int, int, int);
extern int func_8006E7E8(void);
extern void func_8006E1C0(unsigned char *, unsigned char *);

int func_8006BECC(void)
{
    int area;
    int base;
    unsigned char *st;
    int r;
    int k;
    unsigned int i;
    unsigned char *p;
    unsigned char *h;
    unsigned char *q;
    unsigned int w;
    unsigned int f;
    int pad[4];

    area = D_800B0CE2;
    base = D_800B0DD8;
    st = D_800B0CD8;
top:
    {
        switch (st[0xEC]) {
        case 0:
            if (*(int *)st & 0x200000) {
                st[0xEC] = 1;
                goto top;
            }
            goto set6;
        case 1:
            k = area + 3;
            r = func_8006E6A8(base + D_800930D8[k], *(int *)(st + 0x194), D_800930DA[k] - D_800930D8[k]);
            if (r != -1) {
                st[0xEC] = 2;
            }
            return 1;
        case 2:
            r = func_8006E7E8();
            if (r == -1) {
                st[0xEC] = 1;
                return 1;
            }
            if (r != 0) {
                return 1;
            }
            f = *(int *)st;
            if (f & 0x20000) {
                if (!(f & 0x80000) && D_8009D25C < 2) {
                    return 1;
                }
                *(int *)st |= 0x40000;
            }
            st[0xEC] = 3;
            goto top;
        case 3:
            p = *(unsigned char **)(st + 0x194);
            h = p + *(int *)(p + 4);
            q = p + (*(unsigned int *)(h + 0x28) & 0x3FFFFF);
            for (i = 0; i < *(unsigned int *)(h + 0x28) >> 22; i++) {
                func_8006E1C0(q + i * 0x14, p);
            }
            st[0xEC] = 4;
            goto top;
        case 4:
            k = area + 8;
            r = func_8006E6A8(base + D_800930D8[k], *(int *)(st + 0x154), D_800930DA[k] - D_800930D8[k]);
            if (r != -1) {
                st[0xEC] = 5;
            }
            return 1;
        case 5:
            r = func_8006E7E8();
            if (r == -1) {
                st[0xEC] = 4;
                return 1;
            }
            if (r != 0) {
                return 1;
            }
        set6:
            st[0xEC] = 6;
            goto top;
        case 6:
            p = *(unsigned char **)(st + 0x154);
            h = p + *(int *)(p + 4);
            *(unsigned char **)(st + 0x198) = p + (*(unsigned int *)(p + (*(unsigned int *)(h + 0xC) & 0x3FFFFF) + 4) & 0xFFFFFF);
            q = p + (*(unsigned int *)(h + 0x10) & 0x3FFFFF);
            {
                unsigned char *e = q;

                for (i = 0; i < *(unsigned int *)(h + 0x10) >> 22; i++) {
                    *(unsigned char **)(st + 0x1C0 + e[i * 0xC + 7] * 4) = p + (*(unsigned int *)(e + i * 0xC + 4) & 0xFFFFFF);
                }
            }
            st[0xEC] = 0;
            st[0xB] = st[0xA];
            *(int *)st &= ~0x200000;
            return 0;
        default:
            return 0;
        }
    }
}
