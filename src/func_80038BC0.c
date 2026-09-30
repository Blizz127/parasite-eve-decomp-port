/* VRAM 0x80038BC0 / file 0x293C0 / size 0x124. */
extern unsigned char D_80091A1D;
extern unsigned char *D_80091A28;

unsigned char func_80038BC0(unsigned char *base, unsigned char mode) {
    unsigned char *p;
    int i;
    int sel;
    int want;
    unsigned char *q;

    if (base == 0) {
        base = D_80091A28;
    }
    switch (mode) {
    case 0:
        want = 3;
        break;
    case 1:
        want = (D_80091A1D % 10) != 0;
        break;
    case 2:
        want = 2;
        break;
    default:
        want = 4;
        break;
    }
    sel = 0;
    p = base + 1;
    for (i = 0; i < p[2]; i++) {
        if ((p + i)[3] == (unsigned char)want) {
            sel = i;
            i = p[2];
        }
    }
    q = p + 0x1B;
    for (i = 0; i < q[0]; i++) {
        if ((q + i)[1] == (sel & 0xFF)) {
            return i;
        }
    }
    return 0xFF;
}
