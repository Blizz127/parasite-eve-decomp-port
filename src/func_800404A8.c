/* VRAM 0x800404A8 / file 0x30CA8 / size 0xFC. */
typedef struct {
    unsigned char state;
    unsigned char pad[0x417];
} CardState;

extern CardState D_800A0EDC[2];
extern unsigned char D_800A12F4;
extern int D_800A1844;
extern int D_800A1848[2];

int func_800404A8(void) {
    int i;
    register int *p asm("$5");
    int v;

    D_800A1844 -= (D_800A1844 > 0);
    for (i = 0; i < 2; i++) {
        if ((unsigned int)(D_800A0EDC[i].state - 2) < 2) {
            D_800A1844 = 12;
            D_800A1848[i] = 1;
        }
    }
    p = D_800A1848;
    __asm__ volatile("" : "=r"(p) : "0"(p));
    if ((p[0] == 0 || D_800A0EDC[0].state == 4) &&
        (p[1] == 0 || D_800A12F4 == 4)) {
        v = 4;
        if (D_800A1844 < 5) {
            v = D_800A1844;
        }
        D_800A1844 = v;
        p[1] = 0;
        p[0] = 0;
    }
    return D_800A1844 > 0;
}
