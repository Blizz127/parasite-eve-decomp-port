typedef struct Item {
    unsigned char b[32];
} Item;

extern Item D_800C0EAC[];
extern short D_800C1F80[];
extern short D_800923D8[];
extern int D_800A77F0[];

void func_80053128(void)
{
    Item *p;
    short *r;
    register short *q asm("$5");
    int i;
    int k;
    int v;

    for (p = D_800C0EAC; p < D_800C0EAC + 0x80; p++) {
        p->b[5] &= ~8;
    }
    i = 0;
    for (q = D_800C1F80; i < 0x52; i++, q++) {
        v = *q - 0x100;
        if ((unsigned int)v < 0x80) {
            D_800C0EAC[v].b[5] |= 8;
        }
    }
    for (r = D_800923D8; r < D_800923D8 + 0x10; r += 2) {
        for (k = r[0]; k <= r[1]; k++) {
            v = D_800A77F0[k] - 0x100;
            if ((unsigned int)v < 0x80) {
                D_800C0EAC[v].b[5] |= 8;
            }
        }
    }
    for (p = D_800C0EAC; p < D_800C0EAC + 0x80; p++) {
        if (!(p->b[5] & 0x18)) {
            p->b[0] = 0;
        }
        p->b[5] &= ~8;
    }
}
