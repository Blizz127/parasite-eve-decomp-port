extern int D_8009CFC0;
extern signed char D_800C0E22[];
extern int func_80062CC4();
extern unsigned char *func_80062D2C(int, int, int, int);
extern unsigned char *func_8006322C(int, unsigned char *, unsigned char *);
extern void func_80062CB8(unsigned char *);
extern void func_8004D024(int);
extern void func_80052E30(int);
extern unsigned char *func_8005332C(int);
extern void func_800481FC();
extern void func_8004D030();
extern void func_8004FFA8();

void func_800480AC(void)
{
    unsigned char *p;
    unsigned char *q;
    unsigned char *r;
    signed char *s;
    int i;

    p = func_80062D2C(0x28, func_80062CC4(), 0, 1);
    q = func_8006322C(0x28, p, p);
    *(unsigned int *)(p + 0x30) = (unsigned int)func_800481FC;
    *(unsigned int *)(p + 0x2C) = (unsigned int)func_8004D030;
    *(unsigned int *)(q + 0x30) = (unsigned int)func_8004FFA8;
    func_80062CB8(q);
    func_8004D024(0);
    *(int *)(p + 0x34) = 0xDC;
    *(int *)(p + 0x18) = 0x32;
    *(int *)(p + 0x38) += 10;
    *(int *)(q + 0x18) = *(int *)(p + 0x34) - 0x44;
    *(int *)(q + 0x1C) += 8;
    s = D_800C0E22;
    D_8009CFC0 = 0x37;
    if (*s >= 0) {
        func_80052E30(0);
        r = func_8005332C(*s);
        for (i = 0; i < r[0x14]; i++) {
            if (((r + i)[0x15] & 0xE0) == 0xA0) {
                break;
            }
        }
        D_8009CFC0 += ((r + i)[0x15] & 0x1F) - 1;
    }
}
