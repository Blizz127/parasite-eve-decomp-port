/* VRAM 0x80044274 / file 0x34A74 / size 0x1D0.
 *
 * Retail's assembler expands `la` of the small-data words D_8009CF90/94
 * ABSOLUTE (lui/addiu) while the plain store to D_8009CF90 stays
 * gp-relative (0x220($gp)). Both D_8009CF90 and D_8009CF94 are forced
 * absolute (profile env) for the address arguments, and the store is
 * spelled through the neighbouring small word D_8009CF8C so GNU as keeps
 * it gp-relative; the bytes are identical (D_8009CF8C + 4 == D_8009CF90).
 */
extern int D_8009CF88;
extern int D_8009CF8C;
extern int D_8009CF90;
extern int D_8009CF94;

extern int func_80052F0C(void);
extern int func_800562A4(int);
extern void func_80055760(void);
extern unsigned char *func_80062A34(int, int);
extern int func_80063428(unsigned char *);
extern void func_80052E30(int);
extern int func_80052F70(void);
extern int func_80055FE0(int);
extern void func_80062CB8(unsigned char *);
extern void func_8005600C(int *, int *);
extern unsigned char *func_80062CC4(void);
extern void func_800451D0(unsigned char *);

static __inline__ void open_list(void) {
    unsigned char *p;
    unsigned char *q;
    int i;

    p = func_80062A34(2, 0xD);
    q = func_80062A34(2, 0xE);
    if (p != 0 && func_80063428(p) != 0 && q != 0) {
        *(int *)(p + 0x44) = -1;
        func_80052E30(0);
        for (i = 0; i < func_80052F70(); i++) {
            if (func_80055FE0(i) != 0) {
                break;
            }
        }
        *(int *)(q + 0x44) = 0;
        *(int *)(q + 0x48) = (i < func_80052F70()) ? i : 0;
        func_80062CB8(q);
    }
}

void func_80044274(int a0) {
    int r;

    D_8009CF88 = func_80052F0C();
    D_8009CF8C = a0;
    r = func_800562A4(a0);
    if (r == 0) {
        D_8009CF8C = -1;
        func_80055760();
    } else if (r == 1) {
        open_list();
        (&D_8009CF8C)[1] = D_8009CF88;
        func_8005600C(&D_8009CF90, &D_8009CF94);
        func_800451D0(func_80062CC4());
    } else {
        open_list();
    }
}
