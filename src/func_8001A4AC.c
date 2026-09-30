extern void func_8006A318();
extern void func_8001A784();

void func_8001A4AC(unsigned char *a0) {
    int cur;
    int nxt;
    int lim;
    int fl;
    unsigned int f2;

    if (a0 == 0) {
        return;
    }
    {
        unsigned char *c = *(unsigned char **)(a0 + 0x18C);
        if (c != 0) {
            if ((*(int *)(c + 0x98) & 0x800000) == 0) {
                func_8001A4AC(c);
            }
        }
    }
    func_8006A318(a0);
    cur = *(int *)(a0 + 0x14);
    *(int *)(a0 + 0x98) = (*(int *)(a0 + 0x98) & ~8) | 0x800000;
    *(int *)(a0 + 0x18) = cur;
    f2 = *(int *)(a0 + 0x98);
    if (f2 & 0x100) {
        return;
    }
    fl = f2 & 0x200;
    if (fl != 0) {
        if ((unsigned int)cur >> 16 == *(unsigned short *)(a0 + 0x12)) {
            return;
        }
    }
    if (f2 & 0x200000) {
        func_8001A784(a0, *(unsigned char *)(*(unsigned char **)(a0 + 0x18C) + 0xE));
        *(int *)(a0 + 0x14) = *(int *)(*(unsigned char **)(a0 + 0x18C) + 0x14);
        return;
    }
    nxt = cur + *(int *)(a0 + 0x1C);
    lim = *(unsigned short *)(a0 + 0x12) << 16;
    if (fl != 0) {
        if ((cur < lim && lim < nxt) || (lim < cur && nxt < lim)) {
            *(int *)(a0 + 0x14) = lim;
            return;
        }
    }
    {
        int f = *(unsigned char *)(a0 + 0xF);
        int hi = nxt >> 16;
        if (f < hi) {
            nxt = (hi % (f + 1)) << 16;
            cur = 0;
            *(int *)(a0 + 0x98) |= 8;
        } else if (nxt < 0) {
            cur = f << 16;
            nxt = nxt + ((f + 1) << 16);
            *(int *)(a0 + 0x98) |= 8;
        }
    }
    if ((*(int *)(a0 + 0x98) & 0x200) && ((cur <= lim && lim < nxt) || (lim <= cur && nxt < lim))) {
        *(int *)(a0 + 0x14) = lim;
    } else {
        *(int *)(a0 + 0x14) = nxt;
    }
}
