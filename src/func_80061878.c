typedef struct Tag {
    unsigned int addr : 24;
    unsigned int len : 8;
} Tag;

extern unsigned int *D_8009D100;
extern unsigned int D_8009D104;
extern unsigned int *D_8009D11C;
extern void func_800527C0(int);
extern void func_80077C84(unsigned int *, int, int, int);
extern void func_8006153C(int, int, int, int);

void func_80061878(unsigned char *s, int mode)
{
    unsigned int *t1;
    unsigned int *t2;
    int m1;
    int m2;

    t1 = 0;
    m1 = mode + 1;
    if (D_8009D100 + 2 < (unsigned int *)(D_8009D104 + 0x4000)) {
        D_8009D100 = D_8009D100 + 2;
        t1 = D_8009D100 - 2;
    } else {
        func_800527C0(1);
    }
    if (t1 != 0) {
        func_80077C84(t1, 0, 0, (m1 & 3) << 5);
    }
    m2 = 2 - mode;
    t2 = 0;
    if (D_8009D100 + 2 < (unsigned int *)(D_8009D104 + 0x4000)) {
        D_8009D100 = D_8009D100 + 2;
        t2 = D_8009D100 - 2;
    } else {
        func_800527C0(1);
    }
    if (t2 != 0) {
        func_80077C84(t2, 0, 0, (m2 & 3) << 5);
    }
    while ((signed char)s[0] >= 0) {
        func_8006153C((signed char)s[0], (signed char)s[1], 2, mode);
        s += 2;
    }
    s++;
    ((Tag *)t1)->addr = ((Tag *)D_8009D11C)->addr;
    ((Tag *)D_8009D11C)->addr = (unsigned int)t1;
    while ((signed char)s[0] >= 0) {
        func_8006153C((signed char)s[0], (signed char)s[1], -2, mode == 0);
        s += 2;
    }
    ((Tag *)t2)->addr = ((Tag *)D_8009D11C)->addr;
    ((Tag *)D_8009D11C)->addr = (unsigned int)t2;
}
