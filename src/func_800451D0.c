/*
 * func_800451D0 — build the 0x3E window and submit a transformed quad
 * (retail 0x800451D0).
 *
 * VRAM 0x800451D0 / file 0x359D0 / size 0xF0 (60 words). Follows
 * func_80045110 and precedes func_800452C0 (0x35AC0).
 *
 * Allocates window 0x3E, installs func_800452C0 at +0x2C and func_800453E8
 * at +0x30, marks it live, releases/registers the gp-relative quad handles at
 * D_8009CE54 + 0x134/+0x138 and D_8009CF90/D_8009CF94, and — when the selected
 * record's kind byte is outside {0x13,0x14,0x15} — xor-swaps the two handle
 * pairs. Then submits func_80056C40 with the four handles, returns
 * func_80055724() and clears D_8009CFA0 + 0x0C (0x23C($gp)).
 *
 * Build: era -O2 -G8. The xor block must be written as the two classic
 * `a ^= b; b ^= a; a ^= b;` swap sequences: m2c's four-temporary expansion
 * computes the two `t ^ u` terms early and interleaves the chains, shifting
 * three words. Declaration order matters for the gp slots — each containing
 * data symbol is declared as a scalar so the accesses stay gp-relative.
 */
extern int D_8009CE54;
extern int D_8009CF90;
extern int D_8009CF94;
extern int D_8009CFA0;
extern char *func_80062D2C(int, int, int, int);
extern void func_80062CB8(int);
extern int func_8005332C(int);
extern void func_80052E30(int);
extern void func_80056C40(int, int, int, int);
extern int func_80055724(void);
extern void func_800452C0(void);
extern void func_800453E8(void);

int func_800451D0(int arg0) {
    char *p;
    int item;
    int r;

    p = func_80062D2C(0x3E, arg0, 0, 1);
    *(void **)(p + 0x2C) = func_800452C0;
    *(void **)(p + 0x30) = func_800453E8;
    *(int *)(p + 0x28) = 1;
    func_80062CB8((int)p);
    func_80052E30(*(int *)((char *)&D_8009CE54 + 0x134));
    item = func_8005332C(*(int *)((char *)&D_8009CE54 + 0x138));
    if (item != 0 && (unsigned int)(*(unsigned char *)(item + 6) - 0x13) >= 3) {
        *(int *)((char *)&D_8009CE54 + 0x134) ^= *(int *)((char *)&D_8009CF90);
        *(int *)((char *)&D_8009CF90) ^= *(int *)((char *)&D_8009CE54 + 0x134);
        *(int *)((char *)&D_8009CE54 + 0x134) ^= *(int *)((char *)&D_8009CF90);
        *(int *)((char *)&D_8009CE54 + 0x138) ^= *(int *)((char *)&D_8009CF94);
        *(int *)((char *)&D_8009CF94) ^= *(int *)((char *)&D_8009CE54 + 0x138);
        *(int *)((char *)&D_8009CE54 + 0x138) ^= *(int *)((char *)&D_8009CF94);
    }
    func_80056C40(*(int *)((char *)&D_8009CE54 + 0x134),
                  *(int *)((char *)&D_8009CE54 + 0x138),
                  *(int *)((char *)&D_8009CF90),
                  *(int *)((char *)&D_8009CF94));
    r = func_80055724();
    *(int *)((char *)&D_8009CFA0 + 0xC) = 0;
    return r;
}
