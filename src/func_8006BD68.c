extern signed char D_800B0CE0;
extern signed char D_800B0CE1;
extern unsigned char D_800B0CD8;
extern unsigned short D_800930D8[];
extern void func_80074DC0(int a0);
extern int func_8006E6A8(int a0, int a1, int a2);
extern int func_8006E7E8(void);
extern void func_800718D0(int a0);

int func_8006BD68(void)
{
    register unsigned char *g asm("$16");
    register unsigned short *t asm("$17");
    register unsigned short *t2 asm("$19");
    register int m asm("$18");
    register int m2 asm("$17");
    int r;
    int lo;
    int hi;
    int idx;

    g = &D_800B0CD8;
    if (D_800B0CE0 != D_800B0CE1) {
    func_80074DC0(0);
outer:
        t = D_800930D8;
        t2 = t + 1;
        m = -1;
        do {
            idx = (*(signed char *)(g + 8) + 0x2B) * 2;
            lo = *(unsigned short *)(idx + (unsigned int)t);
            hi = *(unsigned short *)(idx + (unsigned int)t2);
            r = func_8006E6A8(*(int *)(g + 0x100) + lo, *(int *)(g + 0x194), hi - lo);
        } while (r == m);
        m2 = -1;
inner:
        r = func_8006E7E8();
        if (r == 0) {
            goto done;
        }
        if (r == m2) {
            goto outer;
        }
        goto inner;
done:
    func_800718D0(*(int *)(g + 0x194));
    g[9] = g[8];
    }
    return 0;
}
