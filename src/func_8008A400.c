extern unsigned int D_800BCD50;
extern unsigned int D_800BCD5C;
extern unsigned int D_8009D2C4;
extern unsigned char D_800BC000[];
extern void func_8008F1B0(unsigned char *a0, unsigned int a1);
extern void func_80089960(void);
extern void func_80089B28(void);
extern void func_80089CF0(void);

void func_8008A400(unsigned int a0, unsigned int a1)
{
    unsigned char *base;
    unsigned int mask;
    unsigned int act;
    unsigned int i;
    int max;
    unsigned int id;

    mask = 0x1000;
    id = a0 & 0xFFFF;
    if (id == 0xFFFF) {
        return;
    }
    base = D_800BC000;
    act = D_800BCD50;
    if (a1 & 0x0FFFFFFF) {
        for (i = 0; i < 12; i++, base += 0x11C, mask <<= 1) {
            if ((act & mask) && (*(unsigned int *)(base + 0x2C) & a1)) {
                if (*(unsigned int *)(base + 0x38) & 0x100000) {
                    *(unsigned int *)(base + 0x38) |= 0x200000;
                } else {
                    D_800BCD5C |= mask;
                    func_8008F1B0(base, mask);
                    *(unsigned int *)(base + 0x38) = 0;
                }
            }
        }
    } else if ((int)a1 < 0) {
        base += id * 0x11C;
        mask <<= id;
        if (act & mask) {
            func_8008A400(*(unsigned int *)(base + 0x28), 0);
        }
        mask <<= 1;
        base += 0x11C;
        if (act & mask) {
            func_8008A400(*(unsigned int *)(base + 0x28), 0);
        }
        return;
    } else if (a1 & 0x40000000) {
        for (i = 0; i < 12; i++, base += 0x11C, mask <<= 1) {
            if (*(unsigned int *)(base + 0x2C)) {
                act &= ~mask;
            }
        }
        base = D_800BC000;
        mask = 0x1000;
        max = 0;
        for (i = 0; i < 12; i++, base += 0x11C, mask <<= 1) {
            if ((act & mask) && max < *(int *)(base + 0x50)) {
                max = *(int *)(base + 0x50);
            }
        }
        base = D_800BC000;
        mask = 0x1000;
        for (i = 0; i < 12; i++, base += 0x11C, mask <<= 1) {
            if ((act & mask) && max == *(int *)(base + 0x50)) {
                if (*(unsigned int *)(base + 0x38) & 0x100000) {
                    *(unsigned int *)(base + 0x38) |= 0x200000;
                } else {
                    D_800BCD5C |= mask;
                    func_8008F1B0(base, mask);
                    *(unsigned int *)(base + 0x38) = 0;
                }
            }
        }
    } else {
        for (i = 0; i < 12; i++, base += 0x11C, mask <<= 1) {
            if ((act & mask) && *(unsigned int *)(base + 0x28) == id) {
                if (*(unsigned int *)(base + 0x38) & 0x100000) {
                    *(unsigned int *)(base + 0x38) |= 0x200000;
                } else {
                    D_800BCD5C |= mask;
                    func_8008F1B0(base, mask);
                    *(unsigned int *)(base + 0x38) = 0;
                }
            }
        }
    }
    D_8009D2C4 |= 0x10;
    func_80089960();
    func_80089B28();
    func_80089CF0();
}
